#include "libTableauNoir.h"
#include <SDL/SDL.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#ifndef __WIN32__
#define __WIN32__ 0
#endif

#ifndef _WIN32
#define _WIN32 0
#endif

/**************************/
/* constantes symboliques */
/**************************/

/* taille de la fenêtre mémoire */
/* compatible norme 4K WHXGA    */
//#define MEM_WIDTH  5120          // Taille max de l'affichage en x
//#define MEM_HEIGHT 3200          // Taille max de l'affichage en y

/* norme HD  */
//#define MEM_WIDTH  1920          // Taille max de l'affichage en x
//#define MEM_HEIGHT 1280          // Taille max de l'affichage en y

static int MEM_WIDTH = 800 ;
static int MEM_HEIGHT = 600 ;

static int SCREEN_WIDTH = 800 ;
static int SCREEN_HEIGHT = 600 ;

/* taille initiale de la fenêtre affichée */
#define WIN_WIDTH_INITIAL 800
#define WIN_HEIGHT_INITIAL 600


/****************************/
/*           types          */
/****************************/

/* type pour les coordonnées */

typedef struct { double x; double y; } tPointF ;
typedef struct { int    x; int    y; } tPoint  ;
typedef struct { unsigned char r; unsigned char v; unsigned char b; } tRVB ;



/**************************/
/* variables statiques    */
/**************************/



// la fenêtre en mémoire fait 1920x1280 pixels (écran Full HD)
// l'origine du repère orthonormé dans la fenêtre mémoire est :
static tPoint s_orig = {0, 0};

// taille de la fenêtre
static tPoint s_taille_win = {WIN_WIDTH_INITIAL, WIN_HEIGHT_INITIAL};


static int s_redimensionnable = 1   ;  /* la fenêtre peut-elle être redimensionnée ? */
static int s_temporise = 0          ;  /* Mode Bufferisation enclenché? (1:non 0:oui) */
static int s_plein_ecran = 0;

static tRVB s_couleur_fond = { 0, 0, 0 };
static tRVB s_couleur_pinceau = { 0xFF, 0xFF, 0xFF } ;
static tRVB s_couleur_stylo = { 0xFF, 0, 0 } ;
static int  s_taille_stylo = 1;            // Diametre du point




static SDL_Surface * s_sdl_win      = NULL;          // Surface visible
static SDL_Surface * s_sdl_mem      = NULL;          // Surface de l'écran virtuel
/*static SDL_Surface * s_sdl_pinceau = NULL;        // Couleur du fond
static SDL_Surface * s_sdl_pixel   = NULL;          // Couleur du pixel (taille: 1*1)*/
static tStylo        s_sdl_stylo    = {NULL, NULL, 0};    // Image d'un point


static EtatSourisClavier lEtat;
static unsigned char s_opacite      = 0xFF  ;       // opacité des tracés
static int s_fermetureDemandee      = 0     ;       // 1 si l'utilisateur essaie de fermer la fenêtre
static int s_attenteBoutonGauche    = 0     ;       // en attente de clic
static int s_attenteBoutonDroit     = 0     ;       // en attente de clic
static int s_attenteBoutonMilieu    = 0     ;       // en attente de clic
static int s_attenteNms             = 0     ;       // en attente chronométrée

/**
 * ortho_mem : traduit les coordonnées dans le repère orthonormé en
 *             coordonnées dans la fenêtre mémoire
 * @param x : abscisse dans le repère orthonormé
 * @param y : ordonnée dans le repère orthonormé
 * @return Coordonnées dans la fenêtre mémoire
 */
tPoint ortho_mem(double x,  double y)
{
    tPoint retour = { s_orig.x + x , s_orig.y - y } ;
    return retour ;
}


tPoint divisionPointParInt( tPoint p, int d ) {
    tPoint retour = { p.x / d , p.y / d } ;
    return retour ;
}

/**
 * ortho_win : traduit les coordonnées dans le repère orthonormé en
 *             coordonnées dans la fenêtre affichée
 * @param x : abscisse dans le repère orthonormé
 * @param y : ordonnée dans le repère orthonormé
 * @return Coordonnées dans la fenêtre affichée
 */
tPoint ortho_win(double x,  double y)
{
    tPoint orig = divisionPointParInt(s_taille_win, 2);
    tPoint retour = { orig.x + x, orig.y - y } ;
    return retour ;
}


/**
 * win_ortho : traduit les coordonnées dans la fenêtre affichée
 *             en coordonnées dans le repère orthonormé
 * @param x : abscisse dans la fenêtre affichée
 * @param y : ordonnée dans la fenêtre affichée
 * @return Coordonnées dans le repère orthonormé
 */
tPoint win_ortho(int x, int y)
{
    tPoint orig = { - s_taille_win.x / 2, s_taille_win.y / 2 };
    tPoint retour = { orig.x + x , orig.y - y } ;
    return retour ;
}

/**
 * win_mem : traduit les coordonnées dans la fenêtre affichée
 *           en coordonnées dans la fenêtre en mémoire
 * @param x : abscisse dans la fenêtre affichée
 * @param y : ordonnée dans la fenêtre affichée
 * @return Coordonnées dans la fenêtre en mémoire
 */
tPoint win_mem(int x, int y)
{
    tPoint orig = { (MEM_WIDTH-s_taille_win.x)/2, (MEM_HEIGHT-s_taille_win.y)/2 };
    tPoint retour = { orig.x + x, orig.y + y } ;
    return retour;
}



SDL_Rect mem_win( SDL_Rect rm )
{
    rm.x += (s_taille_win.x - MEM_WIDTH)  / 2 ;
    rm.y += (s_taille_win.y - MEM_HEIGHT) / 2 ;
    return rm ;
}


void creerTableau(void)
{
    /* Initialisation de la librairie SDL */
    if (s_sdl_win) {
        fprintf( stderr, "Tentative de créer le tableau noir infructueuse : le tableau existe déjà.\n") ;
        return ;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1)                         // Demarrage de la SDL. Si erreur alors...
    {
        fprintf(stderr, "Erreur d'initialisation de la SDL : %s\n", SDL_GetError());    // Ecriture de l'erreur
        return;
    }
    const SDL_VideoInfo * info = SDL_GetVideoInfo();
    MEM_WIDTH = SCREEN_WIDTH  = info -> current_w ;
    MEM_HEIGHT = SCREEN_HEIGHT = info -> current_h ;
    s_orig.x = MEM_WIDTH /2;
    s_orig.y = MEM_HEIGHT/2;
    /* printf("Dimensions maximales : %dx%d\n", MEM_WIDTH, MEM_HEIGHT); */

    /* Initialisation de la fenêtre graphique */

    tableauRedimensionnable( s_redimensionnable ) ;


    SDL_WM_SetCaption("Fenetre libTableauNoir", NULL);              // Nom de la fenetre

    /* Initialisation de la fenêtre graphique mémoire */

    s_sdl_mem = SDL_CreateRGBSurface( SDL_HWSURFACE, MEM_WIDTH, MEM_HEIGHT, 32, 0, 0, 0, 0);

    if (s_sdl_win == NULL || s_sdl_mem == NULL)   // Si l'ouverture échoue
    {
        fprintf(stderr, "Impossible de charger le mode vidéo : %s\n", SDL_GetError());
        fermerTableau() ;
        return ;
    }

    effacerTableau() ;

    /* Initialisation du stylo */

    choisirTypeStylo( 1, 255, 0, 0 );
    if ( s_sdl_stylo.stylo == NULL )
    {
        fprintf(stderr, "Impossible d'initialiser le stylo : %s\n", SDL_GetError());
        fermerTableau() ;
        return ;
    }

    /* initialisation de l'état souris-clavier */
    int i ;
    for(i=0;i<SDLK_LAST;i++)
    {
        lEtat.touchesClavier[i]=0;
    }
    lEtat.sourisBoutonDroit  = 0;
    lEtat.sourisBoutonGauche = 0;
    lEtat.sourisBoutonMilieu = 0;
    lEtat.sourisX            = 0;
    lEtat.sourisY            = 0;

}

void reafficher()                        // Recentre l'origine
{
  if (! s_plein_ecran)
    {
      if(s_redimensionnable)
        {
          s_sdl_win = SDL_SetVideoMode(s_taille_win.x,s_taille_win.y, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);
        }
      else
        {
          s_sdl_win = SDL_SetVideoMode(s_taille_win.x,s_taille_win.y, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
        }
    }
  else 
    {
          s_sdl_win = SDL_SetVideoMode(s_taille_win.x,s_taille_win.y, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    }
  SDL_Rect position = { 0, 0 } ;
  position = mem_win(position);
  SDL_BlitSurface(s_sdl_mem, NULL, s_sdl_win , &position);  // Applique l'image mémoire sur l'écran

  tamponner();
}


void fixerTaille(int largeur, int hauteur)
{
  if( largeur && hauteur )
    {
        s_taille_win.x = largeur ;
        s_taille_win.y = hauteur ;
        s_plein_ecran  = 0 ;
        reafficher() ;
    }
}

void fermerTableau()
{
    if (s_sdl_mem) SDL_FreeSurface(s_sdl_mem);
    if (s_sdl_win) SDL_Quit();
    s_sdl_mem = s_sdl_win = NULL ;
}

void tableauRedimensionnable( int r )
{
    s_redimensionnable = r ;

    if (!s_plein_ecran) {
     
      if(s_redimensionnable)                 // Si le tableau peut être redimensionné
        {
          /* 32: nb de bits de couleur par pixel, Chargement dans carte graphique, redimensionnable, bufferisation */
          s_sdl_win = SDL_SetVideoMode(s_taille_win.x, s_taille_win.y, 32, SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF);
        }
      else
        {
          s_sdl_win = SDL_SetVideoMode(s_taille_win.x, s_taille_win.y, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
        }
    }
}

void tableauPleinEcran()
{
  s_plein_ecran = 1 ;
  s_taille_win.x = SCREEN_WIDTH ;
  s_taille_win.y = SCREEN_HEIGHT;
  reafficher();
}

void creerPoint()       // Dessine le point sur la surface POINT
{
    if (s_taille_stylo == 1)
    {
        if (s_sdl_stylo.temporaire) SDL_FreeSurface(s_sdl_stylo.stylo);
        s_sdl_stylo.stylo = SDL_CreateRGBSurface(SDL_HWSURFACE,1,1, 32, 0, 0, 0, 0);
        SDL_FillRect( s_sdl_stylo.stylo,
                      NULL,
                      SDL_MapRGB(s_sdl_win->format, s_couleur_stylo.r, s_couleur_stylo.v, s_couleur_stylo.b) );

        if (s_sdl_stylo.temporaire) SDL_FreeSurface(s_sdl_stylo.effaceur);
        s_sdl_stylo.effaceur = SDL_CreateRGBSurface(SDL_HWSURFACE,1,1, 32, 0, 0, 0, 0);
        SDL_FillRect( s_sdl_stylo.effaceur,
                      NULL,
                      SDL_MapRGB(s_sdl_win->format, s_couleur_fond.r, s_couleur_fond.v, s_couleur_fond.b) );
    } 
    else 
    {
        unsigned int couleur_stylo = SDL_MapRGB(s_sdl_win->format, s_couleur_stylo.r, s_couleur_stylo.v, s_couleur_stylo.b) ;
        unsigned int couleur_fond  = SDL_MapRGB(s_sdl_win->format, s_couleur_fond.r,  s_couleur_fond.v,  s_couleur_fond.b) ;
        unsigned int transparence_stylo = SDL_MapRGB(s_sdl_win->format, 255^s_couleur_stylo.r, 255^s_couleur_stylo.v, 255^s_couleur_stylo.b) ;
        unsigned int transparence_fond  = SDL_MapRGB(s_sdl_win->format, 255^s_couleur_fond .r, 255^s_couleur_fond .v, 255^s_couleur_fond .b) ;

        if (s_sdl_stylo.temporaire) SDL_FreeSurface(s_sdl_stylo.stylo);
        if (s_sdl_stylo.temporaire) SDL_FreeSurface(s_sdl_stylo.effaceur);
        s_sdl_stylo.stylo    = SDL_CreateRGBSurface(SDL_HWSURFACE, s_taille_stylo, s_taille_stylo, 32, 0, 0, 0, 0);
        s_sdl_stylo.effaceur = SDL_CreateRGBSurface(SDL_HWSURFACE, s_taille_stylo, s_taille_stylo, 32, 0, 0, 0, 0);

        /* couleur de transparence du stylo (autour du disque) */
        SDL_SetColorKey(s_sdl_stylo.stylo, SDL_SRCCOLORKEY, transparence_stylo);
        SDL_FillRect(s_sdl_stylo.stylo,    NULL, transparence_stylo);
        /* couleur de transparence de l'effaceur (autour du disque) */
        SDL_SetColorKey(s_sdl_stylo.effaceur, SDL_SRCCOLORKEY, transparence_fond);
        SDL_FillRect(s_sdl_stylo.effaceur, NULL, transparence_fond );

        // tracé du disque
        int i,j,i2,j2, c=s_taille_stylo/2;
        double r = (double) s_taille_stylo / 2, r2=r*r ;
        for (i=-c; i<=c; i++)
        {
            i2 = i * i;
            for(j=-c; j<=c; j++)
            {
                j2 = j * j;
                if( i2 + j2 <= r2 )
                {
                    SDL_Rect position = { c + i, c + j, 1, 1 } ;
                    SDL_FillRect( s_sdl_stylo.stylo,     &position, couleur_stylo );
                    SDL_FillRect( s_sdl_stylo.effaceur , &position, couleur_fond  );
                }
            }
        }
    }
    fixerOpacite(s_opacite);
    s_sdl_stylo.temporaire = 1 ;
}

// **********************************************************************************
//
// Fonctions des styles et des couleurs.
//
// **********************************************************************************

void choisirCouleurFond(int r,int v,int b)  // Choisie la couleur de fond
{
    if(r<256 && r>=0 && v<256 && v>=0 && b<256 && b>=0)
    {
        s_couleur_fond.r=r;
        s_couleur_fond.v=v;
        s_couleur_fond.b=b;
        creerPoint();
    }
}

void choisirTypeStylo(int taille,int r,int v,int b)     // Choisie la taille du style et le redessine sur la surface POINT
{
    if( taille > 0 )
    {
        s_couleur_stylo.r   = r;
        s_couleur_stylo.v   = v;
        s_couleur_stylo.b   = b;
        // taille arrondie à l'entier impair supérieur
        s_taille_stylo      = taille + 1 - taille % 2 ;
        creerPoint();
    }
}

void choisirCouleurPinceau(int r,int v,int b)           // Choisie la couleur du pinceau
{
    if(r<256 && r>=0 && v<256 && v>=0 && b<256 && b>=0)
    {
        s_couleur_pinceau.r=r;
        s_couleur_pinceau.v=v;
        s_couleur_pinceau.b=b;
    }
}

void fixerOpacite( int opacite )
{
    s_opacite = opacite ;
    SDL_SetAlpha(s_sdl_stylo.stylo,       SDL_SRCALPHA, s_opacite);
    SDL_SetAlpha(s_sdl_stylo.effaceur,    SDL_SRCALPHA, 255      );
}

tStylo stockerStylo()
{
    s_sdl_stylo.temporaire = 0 ;
    return s_sdl_stylo ;
}

void selectionnerStylo( tStylo stylo )
{
    if ( s_sdl_stylo.temporaire ) {
        SDL_FreeSurface( s_sdl_stylo.stylo      );
        SDL_FreeSurface( s_sdl_stylo.effaceur   );
    }
    s_sdl_stylo = stylo ;
    s_taille_stylo = ((SDL_Surface *)stylo.stylo)->w ;
}

void recyclerStylo( tStylo stylo )
{
    if ( s_sdl_stylo.stylo == stylo.stylo )
    {
        s_sdl_stylo.temporaire = 1 ;
    } else {
        SDL_FreeSurface( s_sdl_stylo.stylo      );
        SDL_FreeSurface( s_sdl_stylo.effaceur   );
    }
}



// **********************************************************************************
//
// Fonctions de dessin.
//
// **********************************************************************************

void effacerTableau(void)       // Effacer le tableau
{
    SDL_FillRect(s_sdl_mem, NULL, 
                 SDL_MapRGB(s_sdl_win->format, s_couleur_fond.r, s_couleur_fond.v, s_couleur_fond.b) );
    SDL_FillRect(s_sdl_win, NULL, 
                 SDL_MapRGB(s_sdl_win->format, s_couleur_fond.r, s_couleur_fond.v, s_couleur_fond.b) );
    if (!s_temporise) tamponner();
}


void tracerPoint(double x, double y)
{
    // enregistrement dans le tampon
    tPoint pos = ortho_mem( x, y );
    SDL_Rect position = { pos.x - (s_taille_stylo/2), pos.y - (s_taille_stylo/2), 0, 0 }, temp = position;
    SDL_BlitSurface(s_sdl_stylo.stylo, NULL, s_sdl_mem, &temp);
    // affichage
    position = mem_win(position);
    SDL_BlitSurface(s_sdl_stylo.stylo, NULL, s_sdl_win, &position);
    // double buffer
    if (!s_temporise) tamponner();
}

void effacerPoint(double x, double y)
{
    // enregistrement dans le tampon
    tPoint pos = ortho_mem( x, y );
    SDL_Rect position = { pos.x - (s_taille_stylo/2), pos.y - (s_taille_stylo/2) }, temp = position;
    SDL_BlitSurface(s_sdl_stylo.effaceur, NULL, s_sdl_mem, &temp);
    // affichage
    position = mem_win(position);
    SDL_BlitSurface(s_sdl_stylo.effaceur, NULL, s_sdl_win, &position);
    // double buffer
    if (!s_temporise) tamponner();
}

void tracerSegmentPixelSansTamponner(double x1, double y1, double x2, double y2) // trace une ligne
{
    tPoint pos1 = ortho_mem( x1, y1 );
    tPoint pos2 = ortho_mem( x2, y2 );
    pos1.x -= s_taille_stylo/2 ;
    pos1.y -= s_taille_stylo/2 ;
    pos2.x -= s_taille_stylo/2 ;
    pos2.y -= s_taille_stylo/2 ;

    // Nombre de points à tracer
    double nb_pixel = sqrt((pos2.x-pos1.x)*(pos2.x-pos1.x) + (pos2.y-pos1.y)*(pos2.y-pos1.y) ) + 1;
    double stepx = (pos2.x - pos1.x) / nb_pixel ;
    double stepy = (pos2.y - pos1.y) / nb_pixel ;

    int i;
    for(i=0; i<=nb_pixel; i++)
    {
        // enregistrement dans le tampon
      SDL_Rect position = { pos1.x + i*stepx,   pos1.y + i*stepy }, temp = position;
      SDL_BlitSurface(s_sdl_stylo.stylo, NULL, s_sdl_mem, &temp);
      // affichage
      position = mem_win(position);
      SDL_BlitSurface(s_sdl_stylo.stylo, NULL, s_sdl_win, &position);
    }
}

void tracerSegment(double x1, double y1, double x2, double y2)  // Appel tracerSegmentPixel et test si il faut tamponner.
{
    tracerSegmentPixelSansTamponner( x1, y1, x2, y2 ) ;
    if (!s_temporise) tamponner();
}


void effacerSegmentPixelSansTamponner(double x1, double y1, double x2, double y2) // trace une ligne
{
    tPoint pos1 = ortho_mem( x1, y1 );
    tPoint pos2 = ortho_mem( x2, y2 );
    pos1.x -= s_taille_stylo/2 ;
    pos1.y -= s_taille_stylo/2 ;
    pos2.x -= s_taille_stylo/2 ;
    pos2.y -= s_taille_stylo/2 ;

    // Nombre de points à tracer
    double nb_pixel = sqrt((pos2.x-pos1.x)*(pos2.x-pos1.x) + (pos2.y-pos1.y)*(pos2.y-pos1.y) ) + 1;
    double stepx = (pos2.x - pos1.x) / nb_pixel ;
    double stepy = (pos2.y - pos1.y) / nb_pixel ;

    int i;
    for(i=0; i<=nb_pixel; i++)
    {
        // enregistrement dans le tampon
      SDL_Rect position = { pos1.x + i*stepx,   pos1.y + i*stepy }, temp = position;
        SDL_BlitSurface(s_sdl_stylo.effaceur, NULL, s_sdl_mem, &temp);
        // affichage
        position = mem_win(position);
        SDL_BlitSurface(s_sdl_stylo.effaceur, NULL, s_sdl_win, &position);
    }
}

void effacerSegment(double x1, double y1, double x2, double y2)  // Appel tracerSegmentPixel et test si il faut tamponner.
{
    effacerSegmentPixelSansTamponner( x1, y1, x2, y2 ) ;
    if (!s_temporise) tamponner();
}



void ordonner( double * a, double * b )
{
    if ( *a > *b ) {
        double tmp = *a ;
        *a = *b ;
        *b = tmp ;
    }
}

void tracerRectangle(double x1, double y1, double x2, double y2) // Trace le fond et 4 segment
{
    ordonner( &x1, &x2 ) ;
    ordonner( &y1, &y2 ) ;

    tPoint memPos = ortho_mem( x1, y2 ) ;
    SDL_Surface * pinceau = SDL_CreateRGBSurface(SDL_HWSURFACE,x2-x1,y2-y1, 32, 0, 0, 0, 0);
    SDL_FillRect( pinceau, NULL, SDL_MapRGB(s_sdl_win->format, s_couleur_pinceau.r, s_couleur_pinceau.v, s_couleur_pinceau.b) );
    SDL_SetAlpha( pinceau, SDL_SRCALPHA, s_opacite );

    // enregistrement dans le tampon
    SDL_Rect position = { memPos.x, memPos.y }, temp = position ;
    SDL_BlitSurface(pinceau, NULL, s_sdl_mem, &temp);
    // affichage
    position = mem_win(position);
    SDL_BlitSurface(pinceau, NULL, s_sdl_win, &position);

    SDL_FreeSurface(pinceau);

    tracerSegmentPixelSansTamponner( x1, y1, x1, y2 );
    tracerSegmentPixelSansTamponner( x1, y2, x2, y2 );
    tracerSegmentPixelSansTamponner( x2, y2, x2, y1 );
    tracerSegmentPixelSansTamponner( x2, y1, x1, y1 );

    if (!s_temporise) tamponner();
}


void effacerRectangle(double x1, double y1, double x2, double y2) // Trace le fond et 4 segment
{
    ordonner( &x1, &x2 ) ;
    ordonner( &y1, &y2 ) ;

    tPoint memPos = ortho_mem( x1, y2 ) ;

    SDL_Surface * pinceau = SDL_CreateRGBSurface(SDL_HWSURFACE,x2-x1,y2-y1, 32, 0, 0, 0, 0);
    SDL_FillRect( pinceau, NULL, SDL_MapRGB(s_sdl_win->format, s_couleur_fond.r, s_couleur_fond.v, s_couleur_fond.b) );
    SDL_SetAlpha( pinceau, SDL_SRCALPHA, 255 );

    // enregistrement dans le tampon
    SDL_Rect position = { memPos.x, memPos.y }, temp = position ;
    SDL_BlitSurface(pinceau, NULL, s_sdl_mem, &temp);
    // affichage
    position = mem_win(position);
    SDL_BlitSurface(pinceau, NULL, s_sdl_win, &position);

    SDL_FreeSurface(pinceau);

    effacerSegmentPixelSansTamponner( x1, y1, x1, y2 );
    effacerSegmentPixelSansTamponner( x1, y2, x2, y2 );
    effacerSegmentPixelSansTamponner( x2, y2, x2, y1 );
    effacerSegmentPixelSansTamponner( x2, y1, x1, y1 );

    if (!s_temporise) tamponner();
}





/***************************************************************************/
/*         gestion des évènements                                          */
/***************************************************************************/

static int s_derniereTouche = -1 ;
static int s_attenteTouche = 0 ;

void keyPressEvent( int key )
{
    lEtat.touchesClavier[ key ] = 1 ;
    s_derniereTouche = key ;
    s_attenteTouche = 0 ;
}

void keyReleaseEvent( int key )
{
    lEtat.touchesClavier[ key ] = 0 ;
}

void mouseMotionEvent( int x, int y )
{
    tPoint pos = win_ortho( x, y );
    lEtat.sourisX = pos.x;
    lEtat.sourisY = pos.y;
}

void mouseButtonPressEvent( unsigned char button )
{
    switch (button)
    {
        case SDL_BUTTON_LEFT :
            lEtat.sourisBoutonGauche = 1 ;
            break;

        case SDL_BUTTON_RIGHT :
            lEtat.sourisBoutonDroit  = 1 ;
            break;

        case SDL_BUTTON_MIDDLE :
            lEtat.sourisBoutonMilieu = 1 ;
            break;

        default:
            break;
    }
}

void mouseButtonReleaseEvent( unsigned char button )
{
    switch (button)
    {
        case SDL_BUTTON_LEFT :
            lEtat.sourisBoutonGauche = 0 ;
            s_attenteBoutonGauche = 0 ;
            break;

        case SDL_BUTTON_RIGHT :
            lEtat.sourisBoutonDroit  = 0 ;
            s_attenteBoutonDroit = 0 ;
            break;

        case SDL_BUTTON_MIDDLE :
            lEtat.sourisBoutonMilieu = 0 ;
            s_attenteBoutonMilieu = 0 ;
            break;

        default:
            break;
    }
}


void processEvent( SDL_Event event )
{
    switch( event.type )
    {
        case SDL_KEYDOWN :
            keyPressEvent( event.key.keysym.sym ) ;
            break ;

        case SDL_KEYUP :
            keyReleaseEvent( event.key.keysym.sym ) ;
            break ;

        case SDL_MOUSEMOTION :
            mouseMotionEvent( event.motion.x, event.motion.y ) ;
            break ;

        case SDL_MOUSEBUTTONDOWN :
            mouseButtonPressEvent( event.button.button ) ;
            break;

        case SDL_MOUSEBUTTONUP :
            mouseButtonReleaseEvent( event.button.button ) ;
            break;

        case SDL_USEREVENT :
            s_attenteNms = 0 ;
            break ;

        case SDL_VIDEORESIZE :
            fixerTaille( event.resize.w, event.resize.h ) ;
            break;

//         case SDL_VIDEOEXPOSE :
//             reafficher();
//             break;

        case SDL_QUIT :
            s_fermetureDemandee = 1 ;

        default:
            break;
    }
}

int processEvents(int attendre)
{
    SDL_Event event ;
    if (attendre) {
        if ( SDL_WaitEvent(&event) ) {
            processEvent( event ) ;
            return 1 ;
        }
    } else {
        if ( SDL_PollEvent(&event) ) {
            processEvent( event ) ;
            return 1 ;
        }
    }
    return 0 ;
}



/*******************************************************/
/* Fonctions qui attendent une action de l'utilisateur */
/*******************************************************/

void attendreClicGauche()       // Attend le clic gauche de la souris et retourne ses coordonnées
{
    s_attenteBoutonGauche = 1 ;
    while (s_attenteBoutonGauche)
        processEvents( 1 ) ;
}

void attendreClicDroit()        // idem avec le clic droit
{
    s_attenteBoutonDroit = 1 ;
    while (s_attenteBoutonDroit)
        processEvents( 1 ) ;
}

void attendreClicMilieu(void)       // idem avec le clic du milieu
{
    s_attenteBoutonMilieu = 1 ;
    while (s_attenteBoutonMilieu)
        processEvents( 1 ) ;
}

int attendreTouche(void)            // Attend une touche
{
    s_derniereTouche = -1  ;
    while (s_derniereTouche == -1)
        processEvents( 1 ) ;
    return s_derniereTouche ;
}

Uint32 _fin_attente( Uint32 unused1, void * unused2 )
{
    SDL_Event event = { SDL_USEREVENT } ;
    SDL_PushEvent(&event);
    return 0 ;
}

void attendreNms(int ms)
{
    s_attenteNms = 1 ;
    SDL_AddTimer(ms, _fin_attente, NULL);
    while (s_attenteNms)
        processEvents(1) ;
}

void attendreFermeture(void)
{
  while (!fermetureDemandee())
    processEvents(1);
}

/**************************************/
/*   Fonctions de consultation d'état */
/**************************************/



EtatSourisClavier lireEtatSourisClavier(void)
{
    while (processEvents(0)) {}
    return lEtat;
}

Uint32 getpixel(SDL_Surface *surface,int x,int y)               // Fonction copier sur un site
{
    int bpp = surface->format->BytesPerPixel;               // assez compliqué.
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp; // Je ne saurai pas exactement expliquer

    switch(bpp) {
        case 1:
            return *p;

        case 2:
            return *(Uint16 *)p;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;

        case 4:
            return *(Uint32 *)p;

        default:
            return 0;
    }
}

void lirePixel(int x, int y, int * rouge, int * vert, int * bleu)
{
    Uint8 r,g,b;

    tPoint coord = ortho_mem( x, y ) ;
    SDL_GetRGB( getpixel(s_sdl_mem, coord.x, coord.y), s_sdl_win->format, &r, &g, &b );
    *rouge=r;
    *vert=g;
    *bleu=b;
}


int fermetureDemandee()
{
    return s_fermetureDemandee ;
}




// **********************************************************************************
//
// Fonctions d'images.
//
// **********************************************************************************

Image chargerImage(char *nom)         // charge une image
{
    Image picture;
    picture.image=NULL;
    picture.image = (void*)SDL_LoadBMP(nom);
    if(picture.image == NULL)
        picture.chargement_reussi=0;
    else
        picture.chargement_reussi=1;
    return picture;
}

void fixerCouleurDeTransparence(Image picture,int r,int v,int b)
{
    if(picture.chargement_reussi!=0)
        SDL_SetColorKey((SDL_Surface*)picture.image, SDL_SRCCOLORKEY, SDL_MapRGB(((SDL_Surface*)picture.image)->format, r, v, b));  // La couleur donnée ne sera pas
                                                        //  affichée.
}


void afficherImage(Image picture,int x,int y)                 // Affiche l'image.
{
    tPoint pos = ortho_mem( x, y ) ;

    // enregistrement dans le tampon
    SDL_Rect position = { pos.x, pos.y }, temp = position;
    SDL_SetAlpha( picture.image, SDL_SRCALPHA, s_opacite );
    SDL_BlitSurface( (SDL_Surface*) picture.image, NULL, s_sdl_mem, &temp );
    // affichage
    position = mem_win(position);
    SDL_BlitSurface( (SDL_Surface*) picture.image, NULL, s_sdl_win, &position);

    if (!s_temporise) tamponner();
}

void libererImage( Image image )
{
  if (image.chargement_reussi)
    SDL_FreeSurface((SDL_Surface*) image.image);
}




/***************************************/
/*   Bufferisation (tampon de dessin)  */
/***************************************/

void fixerModeBufferisation(int b)
{
    s_temporise = b;
}

void tamponner()                            // Permet de coller la surface PSEUDO sur ECRAN
{
    SDL_Flip(s_sdl_win);                                      // "tamponne" (double buffer)
}

// **********************************************************************************
//
// Temps
//
// **********************************************************************************

/* Au premier appel, la fonction delta_temps retourne 0.
   A chaque appel suivant, elle retourne le temps écoulé depuis le dernier appel
   en secondes.
*/

float delta_temps()
{
  static Uint32 old = 0;
  if (old==0) old = SDL_GetTicks();
  Uint32 t = SDL_GetTicks();
  float delta = (float) (t-old) / 1000.0f;
  old = t;
  return delta;
}


