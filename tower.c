/*déclaration de fonctionnalités supplémentaires*/
#include <stdlib.h>
#include <stdio.h>
#include "libTableauNoir.h"
#include <stdbool.h> // pour le type booleen
#include <math.h>

/* declaration de constantes utilisateur */
#define HAUTEUR_MAP 1000
#define LARGEUR_MAP 1000
/* Pour interdire de placer les tourelles sur les bords de la map */
#define MARGE_BORDS 100
#define HAUTEUR_ENNEMY 30
#define LARGEUR_ENNEMY 30
#define HAUTEUR_TOURELLE 190
#define LARGEUR_TOURELLE 72
#define NB_MAX_POINT 20
#define NB_MAX_TOURELLE 400
#define NB_MAX_ENNEMY 500
#define TAILLE_CELLULE 50
#define VALEUR_ENNEMY_MORT 10
#define COST_TOURELLE_LASER 100
#define COST_TOURELLE_ZONE 70
#define DAMAGE_LASER 100
#define DAMAGE_ZONE 30
#define RANGE_ZONE 100 //portée de la zone de la tourelle ZONE
#define RANGE_LASER 150 //portée du tir LASER
#define FIRST_HP_ENNEMY 100
#define GOLD 500
//170
#define HP_ENNEMY(wave_c) (100 + 80*(wave_c))
#define NB_ENNEMY_IN_WAVE(wave_c)(3+(wave_c)*2)
#define VITESSE_ENNEMY(wave_c) (50 + (wave_c)*5)

/* macro fonctions */
#define sqr(i) ((i)*(i))
#define max(i,j) (((i)<(j))?(j):(i))
#define min(i,j) (((i)<(j))?(i):(j))
/* Pour placer l'image de l'ennemi centrée sur ses coordionnées */
#define L(i) ((i) - LARGEUR_ENNEMY/2)
#define H(i) ((i) + HAUTEUR_ENNEMY/2)
/* Pour placer l'image de la tourelle centrée sur ses coordionnées */
#define G(i) ((i) - LARGEUR_TOURELLE/2)
#define T(i) ((i) + HAUTEUR_TOURELLE/2)


/* declaration des structure et types utilisateurs */

typedef struct point {
  int x;
  int y;
}point_t;

typedef struct pointf {
  float x;
  float y;
} pointf_t;

typedef struct ennemy{ //on definit un ennemi par ses points de vie, s'il
  int hp;
  int hp_max;          //est actif ou non, son image, sa destination, sa
  bool actif;          //position et sa vitesse.
  Image image;
  int next_point;
  pointf_t pos;
  int v;
}ennemy_t;

typedef enum typetourelle{
  LASER,
  ZONE,
  UPGRADE,
}typetourelle_e;

typedef struct tourelle{
  bool actif;
  point_t pos;
  typetourelle_e typetourelle;
  int range;
  int damage;
  Image image;
  int ennemy_shot; //indice de l'ennemi que l'on vient de shooter
  int shoot_time; //heure du tir
  int cost; //prix d'une tourelle
}tourelle_t;


/* declaration des variables globales */
int nb_points;
float global_time=0;
int last_shoot = 0;
int vie = 4;
int last_ennemy=0;
tStylo s;
tStylo s2;
tStylo s3;
tStylo s4;
int gold; //argent du joueur pour acheter ses tourelles
int nb_ennemy_in_wave = 0;

point_t chemin[NB_MAX_POINT];

ennemy_t horde[NB_MAX_ENNEMY];

tourelle_t pool[NB_MAX_TOURELLE];

int compteur_tourelle=0;
tourelle_t moving_tourelle; // C'est la tourelle qui va se déplacer
bool is_moving_tourelle=false;

int wave_courante=0;//vague courante dans le jeu de 1 à N

/* Images du jeu */
Image ennmy1;
Image ennmy2;
Image ennmy3;
Image ennmy4;
Image tower1;
Image tower2;
Image image_menu;
Image rules;
Image map;
Image Adoor;
Image Ddoor;
Image head_Adoor;
Image head_Ddoor;
Image next_level;
Image next_wave;
Image end;

/* declaration de fonctions utilisateur */
void dessine_chemin();
void time_loop();
void deplace_horde(float);
bool test_souris_clavier(); // retourne false si on quite le jeu
bool test_fin_de_partie();
void test_shoot_ennemy();
void depose_tourelle();
bool identifie_tourelle(EtatSourisClavier);
Image random_image(Image, Image, Image, Image);
void terminer_ennemy(int);
void affiche_ennemy(int);
void efface_ennemy(int);
void wave();
void deplace_ennemy(int, float);
bool souris_dans_image(int, int, EtatSourisClavier);
void affiche_tourelles();
void init_jeu();
void afficherCercle(int, int, int);
int distance(pointf_t, point_t);
bool menu();
bool test_fin_wave();
void afficher_map();
void afficher_fin_map();
bool est_sur_le_chemin(int, int);
void afficher_reference_tourelles();
void depart_ennemy(int,int);
void reset_shot();

void init_jeu()
{
  /* initialisation des ennemis et des tourelles */
  int i=0;
  for (i = 0; i <NB_MAX_ENNEMY; i++)
    horde[i].actif = false;
  for (i = 0; i <NB_MAX_TOURELLE; i++)
    pool[i].actif = false;

  /* initialisation des variables globales */
  global_time=0;
  vie = 4;
  wave_courante = 0;
  last_shoot = 0;
  last_ennemy=0;
  compteur_tourelle = 0;
  nb_ennemy_in_wave = 0;
  gold = GOLD;
  is_moving_tourelle = false;

  /* initialisation du chemin */
  nb_points = 9;
  chemin[0].x = -500;
  chemin[0].y = -125;
  chemin[1].x = -225;
  chemin[1].y = -125;
  chemin[2].x = -225;
  chemin[2].y = -325;
  chemin[3].x = 225;
  chemin[3].y = -325;
  chemin[4].x = 225;
  chemin[4].y = 125;
  chemin[5].x = 25;
  chemin[5].y = 125;
  chemin[6].x = 25;
  chemin[6].y = 225;
  chemin[7].x = 275;
  chemin[7].y = 225;
  chemin[8].x = 275;
  chemin[8].y = 500;
}

/* fonction principale */
int main()
{
  /*initialisation des images */
  ennmy1= chargerImage("ennmy1'.bmp");
  ennmy2= chargerImage("ennmy2'.bmp");
  ennmy3= chargerImage("ennmy3'.bmp");
  ennmy4= chargerImage("ennmy4'.bmp");
  tower1= chargerImage("tower1'.bmp");
  tower2= chargerImage("tower2'.bmp");
  image_menu= chargerImage("menu2.bmp");
  rules= chargerImage("rules2.bmp");
  map= chargerImage("FINALMAP.bmp");
  head_Adoor= chargerImage("arrivee.bmp");
  head_Ddoor= chargerImage("depart.bmp");
  next_wave= chargerImage("nextWave.bmp");
  next_level= chargerImage("nextLevel.bmp");
  end= chargerImage("end.bmp");
  fixerCouleurDeTransparence(ennmy1,0,255,0);
  fixerCouleurDeTransparence(ennmy2,0,255,0);
  fixerCouleurDeTransparence(ennmy3,0,255,0);
  fixerCouleurDeTransparence(ennmy4,0,255,0);
  fixerCouleurDeTransparence(tower1,0,255,0);
  fixerCouleurDeTransparence(tower2,0,255,0);
  fixerCouleurDeTransparence(Adoor,0,255,0);
  fixerCouleurDeTransparence(Ddoor,0,255,0);
  fixerCouleurDeTransparence(head_Adoor,0,255,0);
  fixerCouleurDeTransparence(head_Ddoor,0,255,0);
  fixerCouleurDeTransparence(next_wave,0,255,0);
  fixerCouleurDeTransparence(next_level,0,255,0);
  fixerCouleurDeTransparence(rules,0,255,0);

  creerTableau();
  choisirTypeStylo(50,133,109,77);
  s = stockerStylo();
  choisirTypeStylo(RANGE_ZONE*2, 255,140,0);
  s2 = stockerStylo();
  choisirTypeStylo(2,124,255,10);
  s3 = stockerStylo();
  choisirTypeStylo(3,0,195,255);
  s4 = stockerStylo();

  fixerTaille(LARGEUR_MAP, HAUTEUR_MAP);
  tableauRedimensionnable(0);
  while(1)
    {
      bool m = menu();
      if(m)
	{
	  init_jeu();
	  time_loop();
	}
      else
	{
	  effacerTableau();
	  afficherImage(rules,-500,500);
	  tamponner();
	  attendreClicDroit();
	}
    }
  /*retour du resultat*/
  return EXIT_SUCCESS;
}

/*definiton des fonctions utilisateur*/
bool menu()
{
  EtatSourisClavier esc;

  choisirTypeStylo(20,255,255,255);
  fixerModeBufferisation(1);
  while(1)
    {
      esc = lireEtatSourisClavier();
      if (esc.touchesClavier[97]==1)
	exit(0);
      effacerTableau();
      afficherImage(image_menu,-500,240);
      if((esc.sourisX > -410) && (esc.sourisX < -260) && (esc.sourisY  > -280) && (esc.sourisY < -200))
	{
	  tracerPoint(-410,-240);
	  if(esc.sourisBoutonGauche == 1)
	    {
	      printf("on a cliqué sur PLAY\n");
	      return true;
	    }
	}
      if((esc.sourisX > 230) && (esc.sourisX < 380) && (esc.sourisY  > -280) && (esc.sourisY < -200))
	{
	  tracerPoint(230,-240);
	  if(esc.sourisBoutonGauche== 1)
	    {
	      printf("on a cliqué sur rules\n");
	      return false;
	    }
	}
      tamponner();
    }
}

void dessine_chemin(){
  int i = 0;
  selectionnerStylo(s);
  for(i = 0; i < nb_points - 1; i++)
    {
      tracerSegment(chemin[i].x,chemin[i].y,chemin[i+1].x,chemin[i+1].y);
    }
}



void time_loop()
{
  bool fin = false;
  float dt = delta_temps();
  global_time=0;
  //  while(fin!=0)
  last_ennemy=0;
  fixerModeBufferisation(1);
  //dessine_chemin();
  wave(dt);
  while (!fin)
    {
      // printf("le temps = %d\n",(int)global_time);
      effacerTableau();
      afficher_map();
      //dessine_chemin();
      if (!test_souris_clavier())
	fin = true;

      dt = delta_temps();
      global_time += dt ;
      if(test_fin_wave())
	{
	  reset_shot();
	  wave(dt);
	}
      deplace_horde(dt);
      afficher_reference_tourelles();
      test_shoot_ennemy();
      affiche_tourelles();
      afficher_fin_map();
      tamponner();
      if (test_fin_de_partie())
	{
	  attendreClicGauche();
	  fin = true;
	}
    }
}

void afficher_reference_tourelles()
{
  afficherImage(tower1,G(300),T(-350));
  afficherImage(tower2,G(400),T(-350));
  if(gold >= COST_TOURELLE_LASER)
    {
      selectionnerStylo(s3);
      tracerSegment(260,-390,340,-390);
    }
  else
    {
      if(gold < COST_TOURELLE_LASER)
	{
	  selectionnerStylo(s4);
	  tracerSegment(260,-390,260 +(float)gold/COST_TOURELLE_LASER*80,-390);
	}
    }
  if(gold >= COST_TOURELLE_ZONE)
    {
      selectionnerStylo(s3);
      tracerSegment(360,-390,440,-390);
    }
  else
    {
      if(gold < COST_TOURELLE_ZONE)
	{
	  tracerSegment(360,-390,360+ (float)gold/COST_TOURELLE_ZONE*80,-390);
	}
    }
}



void afficher_map()
{
  afficherImage(map,-500,500);
}

void afficher_fin_map()
{
  afficherImage(head_Adoor,195,500);
  afficherImage(head_Ddoor,-500,-32);
}
void depart_ennemy(int i, int marge)
{
  horde[i].pos.x = chemin[0].x-marge;
  horde[i].pos.y = chemin[0].y;
  horde[i].v = VITESSE_ENNEMY(wave_courante);
  horde[i].image = random_image(ennmy1,ennmy2,ennmy3,ennmy4);
  horde[i].hp = HP_ENNEMY(wave_courante);
  horde[i].hp_max = horde[i].hp;
  horde[i].next_point = 1;
  horde[i].actif = true;
  affiche_ennemy(i);
}

void wave()
{
  int i;
  int delta = 0;
  nb_ennemy_in_wave = NB_ENNEMY_IN_WAVE(wave_courante);
  for (i=0; i<nb_ennemy_in_wave; i++)
	{
	  delta+=100;
	  depart_ennemy(i,delta);
	}
  printf("Puissance ennemi = %d\n",horde[0].hp);
  wave_courante++;
}

void affiche_ennemy(int i)
{
  afficherImage(horde[i].image,L(horde[i].pos.x),H(horde[i].pos.y));
  selectionnerStylo(s3);
  tracerSegment((int)horde[i].pos.x-15,(int)horde[i].pos.y+25,(int)horde[i].pos.x-15+(int)(((float)horde[i].hp/horde[i].hp_max)*30), (int)horde[i].pos.y+25);
}

void efface_ennemy(int i)
{
  //  tracerPoint(horde[i].pos.x,horde[i].pos.y);
}

void recule_ennemy(int i, int nb_pixels)
{
  horde[i].pos.x -= nb_pixels;
}
void deplace_ennemy(int i, float dt)
{
  //printf("deplace ennemy %d\n",i);
  efface_ennemy(i);
  float dh = sqrt(sqr(chemin[horde[i].next_point].x - horde[i].pos.x)+sqr(chemin[horde[i].next_point].y - horde[i].pos.y));
  float dp = horde[i].v*dt;
  if(dp >= dh)
    {
      horde[i].pos.x = chemin[horde[i].next_point].x;
      horde[i].pos.y = chemin[horde[i].next_point].y;
      horde[i].next_point++;
      if(horde[i].next_point>= nb_points)
	{
	  terminer_ennemy(i);
	  return;
	}
    }
  else
    {
      float deltaX,deltaY;
      deltaX= dp*((chemin[horde[i].next_point].x-horde[i].pos.x)/dh);
      deltaY= dp*((chemin[horde[i].next_point].y-horde[i].pos.y)/dh);
      horde[i].pos.x = horde[i].pos.x+ deltaX;
      horde[i].pos.y = horde[i].pos.y+ deltaY;
    }
  affiche_ennemy(i);
}

void terminer_ennemy(int i)
{
  horde[i].actif = false;
  //efface_ennemy(i);

  vie = vie - 1;
  printf("vie perdue\n");

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

#define CELL(X) ((int)((X)/TAILLE_CELLULE)*TAILLE_CELLULE + ((X)>0? (TAILLE_CELLULE/2):(-TAILLE_CELLULE/2)))

bool test_souris_clavier()
{
  static bool LMB_clicked = false;
  static int lastX = 0;
  static int lastY = 0;

  EtatSourisClavier esc;
  esc = lireEtatSourisClavier();
  if (esc.touchesClavier[27]==1)
    return false;
  if (esc.sourisBoutonGauche == 1)
    {
      if(LMB_clicked == true)
	{
	  //printf("le bouton était enfonçé\n");
	  if (is_moving_tourelle)
	    {
	      if (!est_sur_le_chemin(esc.sourisX,esc.sourisY))
		{
		  lastX =  CELL(esc.sourisX);
		  lastY =  CELL(esc.sourisY);
		}
	      afficherImage(moving_tourelle.image,G(lastX),T(lastY));
	    }
	}
      else
	{
	  //printf("le bouton vient d'être enfonçé\n");
	  LMB_clicked = true;
	  identifie_tourelle(esc);
	  if (is_moving_tourelle)
	    {
	      lastX =  CELL(esc.sourisX);
	      lastY =  CELL(esc.sourisY);
	      afficherImage(moving_tourelle.image,G(CELL(esc.sourisX)),T(CELL(esc.sourisY)));
	    }

	}
    }
  else
    {
      if(LMB_clicked == false)
	{}
      else
	{
	  printf("le bouton vien d'être relaché\n");
	  if (is_moving_tourelle)
	    {
	      if (!est_sur_le_chemin(esc.sourisX,esc.sourisY))
		{
		  lastX =  CELL(esc.sourisX);
		  lastY =  CELL(esc.sourisY);
		}
	      depose_tourelle(moving_tourelle,lastX,lastY);
	      is_moving_tourelle = false;
	    }
	  LMB_clicked = false;
	}
    }
  return true;
}

bool test_fin_de_partie(){
  if(vie <= 0)
  {
    afficherImage(end,-500,500);
    tamponner();
    return true;
  }
  return false;
}

int distance(pointf_t n, point_t p)
{
  int dis;
  dis = sqrt(sqr(n.x-p.x) + sqr(n.y - p.y));
  return dis;
}


void test_shoot_ennemy()
{

  if ((int)global_time >= last_shoot + 2)
    {
      //printf("Shoot !\n");
      last_shoot = (int)global_time;
      int k,j,l,m;
      for( k = 0; k < compteur_tourelle; k++)
	{
	  if((pool[k].actif == true) && (pool[k].typetourelle == ZONE))
	    {

	      for (j=0; j < nb_ennemy_in_wave;j++)
		{
		  if((horde[j].actif == true) && ((distance(horde[j].pos, pool[k].pos)) <= pool[k].range))
		    {
		      horde[j].hp = horde[j].hp - pool[k].damage;
		      afficherCercle(pool[k].pos.x, pool[k].pos.y, k);
		      pool[k].ennemy_shot = j;
		      if(horde[j].hp <= 0)
			{
			  horde[j].actif = false;
			  gold = gold + VALEUR_ENNEMY_MORT;
			}
		    }
		}
	    }
	  else
	    if((pool[k].actif==true)&&(pool[k].typetourelle==LASER))
	      {
#define GROS_ENTIER 100000
		int plus_proche = 0;
		int dist,dist_min;
		dist_min = GROS_ENTIER;
		for (l=0; l < nb_ennemy_in_wave;l++)
		  {
		    if(horde[l].actif == true)
		      {
			dist = distance(horde[l].pos, pool[k].pos);
			if (dist < dist_min)
			  {
			    dist_min = dist;
			    plus_proche = l;
			  }
		      }
		  }
		if (dist_min < GROS_ENTIER && dist_min <= pool[k].range )
		  {

		    selectionnerStylo(s4);
		    tracerSegment(pool[k].pos.x,pool[k].pos.y+40,horde[plus_proche].pos.x, horde[plus_proche].pos.y);
		    pool[k].shoot_time = global_time;
		    pool[k].ennemy_shot = plus_proche;
		    horde[plus_proche].hp = horde[plus_proche].hp - pool[k].damage;
		    if(horde[plus_proche].hp <= 0)
		      {
			horde[plus_proche].actif = false;
			gold = gold + VALEUR_ENNEMY_MORT;
		      }
		  }
	      }
	}
    }
}

bool test_fin_wave()
{
  int i;
  int num;
  bool wave_termine=true;
  for (i = 0 ; i < nb_ennemy_in_wave; i++)
    {

      if (horde[i].actif)
	{
	  wave_termine = false;
	  break;
	}
    }
  if (wave_termine)
    {
      afficherImage(next_wave,-250,200);
      printf("wave terminé ! Passage à la wave %d\n",wave_courante);
      tamponner();
      attendreNms(2000);
    }
  return wave_termine;
}


void depose_tourelle(tourelle_t t, int x, int y)
{
  t.pos.x = x;
  t.pos.y = y;
  t.actif = true;
  pool[compteur_tourelle] = t;
  gold -= pool[compteur_tourelle].cost;
  compteur_tourelle++;
}

bool souris_dans_image(int CentreX, int CentreY, EtatSourisClavier esc)
{
    if (esc.sourisX >= CentreX - LARGEUR_TOURELLE / 2 &&
      esc.sourisX <= CentreX + LARGEUR_TOURELLE / 2 &&
      esc.sourisY >= CentreY - HAUTEUR_TOURELLE / 2 &&
      esc.sourisY <= CentreY + HAUTEUR_TOURELLE /2 )
          return true;
  else
     return false;
}

bool identifie_tourelle(EtatSourisClavier esc)
{
  tourelle_t t;
  if (souris_dans_image(300, -350, esc) && (gold >= COST_TOURELLE_LASER))
 {
      t.typetourelle = LASER;
      t.range = RANGE_LASER;
      t.actif = false;
      t.damage = DAMAGE_LASER;
      t.image = tower1;
      t.ennemy_shot = 0;
      t.shoot_time = 0;
      t.cost = COST_TOURELLE_LASER;
      printf("la tourelle identifiée est la 1\n");
      is_moving_tourelle = true;
      moving_tourelle = t;
      return true;
    }
  if (souris_dans_image(400, -350, esc)&& (gold >= COST_TOURELLE_ZONE))
    {
      t.typetourelle = ZONE;
      t.range = RANGE_ZONE;
      t.actif = false;
      t.damage = DAMAGE_ZONE;
      t.image = tower2;
      t.ennemy_shot = 0;
      t.shoot_time = 0;
      t.cost = COST_TOURELLE_ZONE;
      printf("la tourelle identifiée est la 2\n");
      moving_tourelle = t;
      is_moving_tourelle = true;
      return true;
    }
  return false;
}

void affiche_tourelles()
{
  int i;
  for (i=0; i < compteur_tourelle; i++)
    {
      if (pool[i].actif)
	{
	  afficherImage(pool[i].image,G(pool[i].pos.x),T(pool[i].pos.y));
	  if(global_time<pool[i].shoot_time+2 && horde[pool[i].ennemy_shot].actif)
	    {
	      selectionnerStylo(s4);
	      tracerSegment(pool[i].pos.x,pool[i].pos.y+40,horde[pool[i].ennemy_shot].pos.x, horde[pool[i].ennemy_shot].pos.y);
	    }
	}
    }
}

void afficherCercle(int Xcentre, int Ycentre,int i)
{
  selectionnerStylo(s2);
  tracerPoint(Xcentre,Ycentre);
}

bool est_sur_le_chemin(int x, int y)
{
  int k = 0;
  int i = 0;
  int j = 0;
  for(i = 0; i < nb_points - 1; i++)
    {
      int m = TAILLE_CELLULE/2;
      if (x >= min(chemin[i].x,chemin[i+1].x)-m && x <= max(chemin[i].x,chemin[i+1].x)+m
	  && y >= min(chemin[i].y,chemin[i+1].y)-m && y <= max(chemin[i].y,chemin[i+1].y)+m)
	return true;
    }
  if (x < - (LARGEUR_MAP/2)+MARGE_BORDS ||
      x >  (LARGEUR_MAP/2) -MARGE_BORDS ||
      y < - (HAUTEUR_MAP/2)+MARGE_BORDS ||
      y >  (HAUTEUR_MAP/2) -MARGE_BORDS)
    {return true;}

  //boucle pour que les tourelles ne se supperposent pas
  for(k=0; k < compteur_tourelle;k++)
    {
      if(CELL(x) == pool[k].pos.x && CELL(y) == pool[k].pos.y)
	{
	  return true;
	}
    }
  return false;
}

void reset_shot()
{
  int i;
  for (i = 0; i < compteur_tourelle;i++)
    {
      pool[i].shoot_time = 0;
    }
}
