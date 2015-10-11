#ifndef _TABLEAUNOIR_H
#define _TABLEAUNOIR_H


    /******************************************/
    /* structure de données pour communiquer  */
    /* l'état du clavier et de la souris      */
    /******************************************/

typedef struct {
    int sourisBoutonGauche;     /* 1 si le bouton gauche est enfoncé, 0 sinon */
    int sourisBoutonDroit;          /* Idem pour le bouton droit */
    int sourisBoutonMilieu;         /* Idem pour le bouton du milieu (roulette) */
    int sourisX;                    /* Position du pointeur de la souris en X. */
    int sourisY;                    /* Position du pointeur de la souris en Y. */

    char touchesClavier[500];    /* Tableau des touches enfoncées (1 pour une
                                   touche enfoncée, 0 sinon)
                                    L'index d'une touche correspond au code
                                    ASCII de la lettre minuscule inscrite
                                    dessus (pour les lettres, bien sûr).
                                    Tableau de 500 cases.*/
} EtatSourisClavier;


    /***************************************/
    /* structure de données permettant de  */
    /* stocker une image en mémoire        */
    /* avant de l'afficher                 */
    /***************************************/

typedef struct
{
    int chargement_reussi;          // Égal a 1 si le chargement a réussi, 0 sinon.
    void *image;                    // Champ propre à libTableauNoir. Vous n'aurez pas a l'utiliser.

} Image;

    /****************************************/
    /* structure permettant de stocker les  */
    /* stylos                               */
    /****************************************/

typedef struct
{
    void * stylo    ;                   // Champ propre à libTableauNoir. Vous n'aurez pas a l'utiliser.
    void * effaceur ;                   // Champ propre à libTableauNoir. Vous n'aurez pas a l'utiliser.
    int temporaire  ;                   // Champ propre à libTableauNoir. Vous n'aurez pas a l'utiliser.
} tStylo ;

// **********************************************************************************
//
// Fonctions indispensables.
//
// **********************************************************************************

    /*************************************/
    /* Initialisation de la bibliothèque */
    /*************************************/
    /* Fonction à appeler en premier     */
    /* pour créer la fenêtre graphique   */

    void creerTableau();


    /* fixe la taille du tableau       */
    /* valeur par défaut : (1920,1080) */

    void fixerTaille(int largeur, int hauteur);

    /* affichage en plein écran */
    void tableauPleinEcran();
    
    /* rend la fenêtre redimensionnable par l'utilisateur
       ou non.
        paramètre : 1 pour la rendre redimensionnable par l'utilisateur
                    0 pour empêcher l'utilisateur de la redimensionner
    */
    void tableauRedimensionnable( int r );

    /* fermeture de la fenêtre graphique */
    void fermerTableau() ;





// **********************************************************************************
//
// Fonctions des styles et des couleurs.
//
// **********************************************************************************

    /* choix de la couleur du fond
    paramètres : composantes de couleur Rouge,Vert,Bleu (entre 0 et 255) */

    void choisirCouleurFond( int rouge, int vert, int bleu );

    /* création du stylo utilisé par défaut
       paramètres :
    1 - largeur de la pointe
    2,3,4 - composantes de couleur Rouge,Vert,Bleu (entre 0 et 255) */

    void choisirTypeStylo( int taille, int rouge, int vert, int bleu );

    /* choix de la couleur de remplissage des figures pleines
    paramètres : composantes de couleur Rouge,Vert,Bleu (entre 0 et 255) */

    void choisirCouleurPinceau( int rouge, int vert, int bleu );

    /* Choix de l'opacité des tracés
       L'opacité est comprise entre 0 (tracé complètement transparent = invisible)
       et 255 (tracé complètement opaque).
       Seul le stylo courant est affecté. */

    void fixerOpacite( int opacite );

    /* Sauvegarde du stylo courant. */
    /* Comme créer un stylo avec choisirTypeStylo prend beaucoup de temps, il est
    préférable d'enregistrer les caractéristiques des stylos lorsque l'on en change
    souvent. Le changement de stylo se fait alors avec la fonction selectionnerStylo.
    Un stylo sauvé en mémoire doit être recyclé lorsqu'il n'est plus utile. */

    tStylo stockerStylo() ;

    /* sélection d'un stylo précédemment créé et sauvegardé */

    void selectionnerStylo( tStylo stylo );

    /* recyclage d'un stylo */

    void recyclerStylo( tStylo stylo );


// **********************************************************************************
//
// Fonctions de dessin.
//
// **********************************************************************************

    /* Effacer la fenêtre */

    void effacerTableau();

    /* tracer un point               */
    /* paramètres : x et y du point  */

    void tracerPoint( double x, double y );

    /* tracer un point de la couleur du fond */
    /* paramètres : x et y du point          */

    void effacerPoint( double x, double y );

    /* tracer un segment de droite                                  */
    /* paramètres : x et y du premier point, x et y du second point */

    void tracerSegment( double x1, double y1, double x2, double y2 );

    /* tracer un segment de droite de la couleur du fond            */
    /* paramètres : x et y du premier point, x et y du second point */

    void effacerSegment( double x1, double y1, double x2, double y2 );

    /* tracer un rectangle                                           */
    /* paramètres : x et y d'un des sommets, x et y du sommet opposé */

    void tracerRectangle( double x1, double y1, double x2, double y2 );

    /* tracer un rectangle de la couleur du fond                     */
    /* paramètres : x et y d'un des sommets, x et y du sommet opposé */

    void effacerRectangle( double x1, double y1, double x2, double y2 );


    /*******************************************************/
    /* Fonctions qui attendent une action de l'utilisateur */
    /*******************************************************/

    /* suspend l'exécution jusqu'à ce que l'utilisateur clique sur la fenêtre */
    /* avec le bouton de gauche                                               */

    void attendreClicGauche();

    /* suspend l'exécution jusqu'à ce que l'utilisateur clique sur la fenêtre */
    /* avec le bouton de droite                                               */

    void attendreClicDroit();

    /* suspend l'exécution jusqu'à ce que l'utilisateur clique sur la fenêtre */
    /* avec le bouton du milieu                                               */

    void attendreClicMilieu();

    /* suspend l'exécution jusqu'à l'appui d'une touche                       */
    /* valeur de retour : le caractère correspondant à la touche enfoncée     */

    int attendreTouche();

    /* suspend l'exécution pendant n millisecondes */
    /* paramètre : le nombre de millisecondes      */

    void attendreNms( int ms );

    /* suspend l'exécution jusqu'à ce que l'utilisateur */
    /* ferme la fenêtre du tableau noir                 */

    void attendreFermeture() ;


    /**************************************/
    /*   Fonctions de consultation d'état */
    /**************************************/


    /* demander l'état de la souris et du clavier */

    EtatSourisClavier lireEtatSourisClavier(void);

    /* lire la couleur d'un pixel                                                   */
    /* paramètre : les coordonnées du pixel                                         */
    /*             des adresses de trois entiers, qui contiendront à la sortie      */
    /*                 les valeurs des composantes rouge, verte et bleue            */
    /*                 de la couleur du pixel                                       */

    void lirePixel(int x, int y, int * rouge, int * vert, int * bleu);

    /* consultation de l'état de la fenêtre graphique :
        renvoie 1 si l'utilisateur a cliqué sur le bouton de fermeture de la fenêtre
        renvoie 0 sinon
    */

    int fermetureDemandee() ;


// **********************************************************************************
//
// Fonctions d'images.
//
// **********************************************************************************

    /* Chargement d'une image en mémoire, depuis un fichier.
       paramètre : le chemin d'accès au fichier image
       valeur de retour : une structure permettant d'accéder à l'image en mémoire
                          pour l'afficher
       Si l'image a bien été chargée, le champ chargement_reussi de la structure retournée
       contient 1, sinon 0.

       Attention, le fichier contenant l'image doit être au format BitMap (extension .bmp) */

    Image chargerImage( char * nom_fichier );

    /* Définition d'une "couleur de transparence" pour une image
       Lors de l'affichage de l'image, les pixels de cette couleur seront ignorés
       paramètres : l'image et les composantes (rouge, vert, bleu) de la couleur à ignorer */

    void fixerCouleurDeTransparence( Image image, int rouge, int vert, int bleu );

    /* Affichage d'une image chargée en mémoire
       paramètres : l'image et les coordonnées du point supérieur gauche où afficher l'image */

    void afficherImage( Image image, int x, int y );

    /* Libération de la mémoire occupée par une image chargée.
       A utiliser lorsqu'on en a fini avec une image et qu'on n'aura plus l'occasion
        de l'afficher */

    void libererImage( Image image ) ;


// **********************************************************************************
//
// Bufferisation (tampon de dessin)
//
// **********************************************************************************

    /* Pour fluidifier l'affichage, on peut utiliser
    la bufferisation pour que l'affichage ne soit pas modifié
    au moment de l'appel d'une fonction de dessin, mais seulement
    lors de l'appel à la fonction "tamponner" : tous les dessins
    effectués avant l'appel à cette fonction deviennent visibles
    à ce moment */

    /* faut-il utiliser le mode bufferisation ?                     */
    /* paramètre :  1 pour oui, 0 pour non                          */

    void fixerModeBufferisation( int unOuZero );

    /* affichage à l'écran et vidage du contenu du tampon */

    void tamponner();


// **********************************************************************************
//
// Temps
//
// **********************************************************************************

    /* Au premier appel, la fonction delta_temps retourne 0.
       A chaque appel suivant, elle retourne le temps écoulé depuis le dernier appel
       en secondes.
    */
    float delta_temps();

#endif
