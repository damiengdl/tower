/*déclaration de fonctionnalités supplémentaires*/
#include <stdlib.h>
#include <stdio.h>
#include "libTableauNoir.h"
#include <stdbool.h> // pour le type booleen
#include <math.h>
#include <SDL/SDL.h>

/* declaration de constantes et types utilisateurs */
#define NB_MAX_POINT 20
#define NB_MAX_TOURELLE 50
#define NB_MAX_ENNEMY 100
#define sqr(i) ((i)*(i))

bool fin;
int nb_points;
float global_time=0;
bool LMB_clicked;
int vie = 20;
//pp
int last_ennemy=0;
tStylo s;


typedef struct point {
  int x;
  int y;
}point_t;
point_t chemin[NB_MAX_POINT];

typedef struct ennemy{ //on definit un ennemie par ses points de vie, s'il 
  int hp;              //est actif ou non, son image, sa destination, sa 
  bool actif;          //position et sa vitesse.
  Image image;
  int next_point;
  point_t pos;
  int v;
}ennemy_t;
ennemy_t horde[NB_MAX_ENNEMY];


typedef enum typetourelle{
  LASER,
  ZONE,
  UPGRADE,
}typetourelle_e;

typedef struct tourelle{
  //pp
  bool actif;
  point_t pos;
  typetourelle_e typetourelle;
  int range;
  int damage;
  Image image;
}tourelle_t;
tourelle_t pool[NB_MAX_TOURELLE];

typedef struct zone_tourelle_type{
  point_t haut_gauche;
  point_t bas_droit;
}zone_tourelle_t;

Image ennmy1;
Image ennmy2;
Image ennmy3;
Image ennmy4;
Image tower1;
Image tower2;

//pp
int hauteur_map=1000;
int largeur_map=1000;
#define X(i) ((i) - largeur_map/2)
#define Y(i) ((i) - hauteur_map/2)

#define invX(i) ((i) + largeur_map/2)
#define invY(i) ((i) + hauteur_map/2)

int hauteur_ennemy=30;
int largeur_ennemy=30;
#define L(i) ((i) - largeur_ennemy/2)
#define H(i) ((i) + hauteur_ennemy/2)

int hauteur_tourelle = 120;
int largeur_tourelle = 72;
//ppp
#define G(i) ((i) - largeur_tourelle/2)
#define T(i) ((i) + hauteur_tourelle/2)

/* declartion de fonctions utilisateur */
void dessine_chemin();
void time_loop();
void deplace_horde(float);
void test_souris_clavier();
void test_fin_de_partie();
void test_shoot_ennemy();
void depose_tourelle();
tourelle_t identifie_tourelle(EtatSourisClavier);
Image random_image(Image, Image, Image, Image);
void terminer_ennemy(int);
void affiche_ennemy(int);
void efface_ennemy(int);
void wave(float dt);
void deplace_ennemy(int i, float dt);
bool souris_dans_image(Image image, int CentreX, int CentreY, EtatSourisClavier esc);

/* fonction principale */
int main()
{
  /*initialisation des variables globales */

  ennmy1= chargerImage("ennmy1'.bmp");
  ennmy2= chargerImage("ennmy2'.bmp");
  ennmy3= chargerImage("ennmy3'.bmp");
  ennmy4= chargerImage("ennmy4'.bmp");
  tower1= chargerImage("tower1'.bmp");
  tower2= chargerImage("tower2.bmp");
  fixerCouleurDeTransparence(ennmy1,0,255,0);
  fixerCouleurDeTransparence(ennmy2,0,255,0);
  fixerCouleurDeTransparence(ennmy3,0,255,0);
  fixerCouleurDeTransparence(ennmy4,0,255,0);
  fixerCouleurDeTransparence(tower1,0,255,0);
  fixerCouleurDeTransparence(tower2,0,255,0);



  /*declaration des variables locales*/
  creerTableau();

  LMB_clicked = false;
  nb_points = 6;
  // pp inutile 
  // chemin[nb_points];
  chemin[0].x = -500;
  chemin[0].y = 300;
  chemin[1].x = 0;
  chemin[1].y = 300;
  chemin[2].x = 400;
  chemin[2].y = 300;
  chemin[3].x = 400;
  chemin[3].y = 100;
  chemin[4].x = 700;
  chemin[4].y = 100;
  chemin[5].x = 700;
  chemin[5].y = 1000;
  choisirTypeStylo(50,133,109,77);
  s = stockerStylo();
  //pp
  fixerTaille(largeur_map, hauteur_map);
  tableauRedimensionnable(0);
  int i=0;
  for (i = 0; i <NB_MAX_ENNEMY; i++)
    horde[i].actif = false;
  for (i = 0; i <NB_MAX_TOURELLE; i++)
    pool[i].actif = false;

  /*calculs effectues par la fonction*/
//ppp
//  dessine_chemin();

  //pp
  fin=0;
  time_loop();
  /*retour du resultat*/
  return EXIT_SUCCESS;
}

/*definiton des fonctions utilisateur*/
  void dessine_chemin(){
    int i = 0;
    for(i = 0; i < nb_points - 1; i++)
      {
	selectionnerStylo(s);
	tracerSegment(X(chemin[i].x),Y(chemin[i].y),X(chemin[i+1].x),Y(chemin[i+1].y));
      }
  }

void time_loop()
{
  // pp
  global_time=0;
  //  while(fin!=0)
  last_ennemy=0;
  fixerModeBufferisation(1);
  while (!fin)
    {
      float dt = delta_temps();
      global_time += dt ;      
      printf("le temps = %d\n",(int)global_time);
      test_souris_clavier();
      //test_shoot_ennemy();
      //pp
      effacerTableau();
      //dessine_chemin();
      wave(dt);
      deplace_horde(dt);     
      afficherImage(tower1,X(G(800)),Y(T(150)));
      afficherImage(tower2,X(G(900)),Y(T(150)));
      tamponner();
      //test_fin_de_partie();
      //pp
      //attendreClicGauche();
      //printf("le temps = %d\n",(int)global_time);
    }
}

void depart_ennemy(int i)
{
  horde[i].pos = chemin[0];
  horde[i].v = 30;
  horde[i].image = random_image(ennmy1,ennmy2,ennmy3,ennmy4);
  horde[i].hp = 50;
  horde[i].next_point = 1;
  horde[i].actif = true;
  affiche_ennemy(i);
}

//pp
void wave(float dt)
{
  static bool wave_en_attente = true;
  int delta = 0;
  if ((int)global_time % 60 == 0)
    {
      if (wave_en_attente)
	{ 
	  wave_en_attente = false;
	  int i = 0;
	  for (i=last_ennemy; i<last_ennemy + 10; i++)
	    {
	      depart_ennemy(i);
	      deplace_ennemy(i, (float)delta);
	      delta+=2;
	    }
	  last_ennemy+=10;
	}
    }
  else
    {
      wave_en_attente = true;
    }
}

void affiche_ennemy(int i)
{
  afficherImage(horde[i].image,X(L(horde[i].pos.x)),Y(H(horde[i].pos.y)));
}


void deplace_ennemy(int i, float dt)
{
  //printf("deplace ennemy %d\n",i);
  //efface_ennemy(i);
  float dh = sqrt(sqr(chemin[horde[i].next_point].x - horde[i].pos.x)+sqr(chemin[horde[i].next_point].y - horde[i].pos.y));
  float dp = horde[i].v*dt;
  if(dp >= dh)
    {
      horde[i].pos = chemin[horde[i].next_point];
      horde[i].next_point++;
      if(horde[i].next_point>= nb_points)
	{
	  terminer_ennemy(i);
	  return;
	}
    }
  else
    {
      int deltaX,deltaY;
      if (chemin[horde[i].next_point].x-horde[i].pos.x > 0)
	deltaX= ceil(dp*((chemin[horde[i].next_point].x-horde[i].pos.x)/dh));
      else
	deltaX= floor(dp*((chemin[horde[i].next_point].x-horde[i].pos.x)/dh));

      if (chemin[horde[i].next_point].y-horde[i].pos.y > 0)
	deltaY= ceil(dp*((chemin[horde[i].next_point].y-horde[i].pos.y)/dh));
      else
	deltaY= floor(dp*((chemin[horde[i].next_point].y-horde[i].pos.y)/dh));

      horde[i].pos.x = horde[i].pos.x+ deltaX;
      horde[i].pos.y = horde[i].pos.y+ deltaY;
    }      
  affiche_ennemy(i);
}

void terminer_ennemy(int i)
{
  horde[i].actif = false;
  //efface_ennemy(i);
}

int alea(int a, int b){
  return a + (b-a) * ((float)rand())/RAND_MAX;
}


Image random_image(Image i, Image j, Image k, Image l)
{
  int o;
 o = alea(1,5);
 if(o==1)
   {return i;}
 else
   {
     if(o == 2)
       {return j;}
     else
       {
	 if(o == 3)
	   {return k;}
	 else
	   {return l;}
       }
   }
}

void deplace_horde(float dt)
{
  int i;
  //  printf("deplace_horde %f\n",dt);
  for(i=0;i<NB_MAX_ENNEMY;i++)
    {
      if (horde[i].actif)
	{
	  deplace_ennemy(i,dt);
	}
    }
}

void test_souris_clavier()
{
  EtatSourisClavier esc;
  esc = lireEtatSourisClavier();
  if (esc.touchesClavier[97]==1)
    exit(0);
  if (esc.sourisBoutonGauche == 1)
    {
      if(LMB_clicked == true)
	{}
      else
	{
	  LMB_clicked = true;
	}
    }
  else
    {
      if(LMB_clicked == false)
	{}
      else
	{
	  LMB_clicked = false;
	}
    }

  if(LMB_clicked == true)
    {
      identifie_tourelle(esc);
    }
  else
    {	 
      depose_tourelle();
    }
  identifie_tourelle(esc);  
}

void test_fin_de_partie(vie){}
/* if(vie = 0)
    {
      dt == dt;
      //afficher game over en fondu si possible
    }
    }
*/

void test_shoot_ennemy(){}


void depose_tourelle(tourelle_t t)
{
  EtatSourisClavier esc;
  esc = lireEtatSourisClavier();
  t.pos.x = esc.sourisX;
  t.pos.y = esc.sourisY;
  t.actif = true;

}

bool souris_dans_image(Image image, int CentreX, int CentreY, EtatSourisClavier esc)
{
  if (image.image)
    {
      SDL_Surface *surf = (SDL_Surface*)image.image;
      if (esc.sourisX >= X(CentreX) - surf->w / 2 &&
	  esc.sourisX <= X(CentreX) + surf->w / 2 &&
	  esc.sourisY >= Y(CentreY) - surf->h / 2 &&
	  esc.sourisY <= Y(CentreY) + surf->h /2 )
	return true;
    }
  return false;
}

tourelle_t identifie_tourelle(EtatSourisClavier esc)
{
  tourelle_t t;
  if (souris_dans_image(tower1, 800, 150, esc))  
 {
      t.typetourelle = LASER;
      t.range = 50;
      t.actif = false;
      t.damage = 50;
      t.image = tower1; 
      printf("la tourelle identifiée est la 1\n");
      return t;
    }
  if (souris_dans_image(tower2, 900, 150, esc))  
    {
      t.typetourelle = ZONE;
      t.range = 50;
      t.actif = false;
      t.damage = 10;
      t.image = tower2;
      printf("la tourelle identifiée est la 2\n");
      return t;
    }
}
