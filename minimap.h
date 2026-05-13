#ifndef MINIMAP_H_INCLUDED
#define MINIMAP_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "perso.h"

/**
* @struct Minimap
* @brief struct for minimap
*/
typedef struct
{
	SDL_Surface* img;
	SDL_Rect pos;
	SDL_Surface* perso_img;
	SDL_Rect perso_pos;
}minimap;

/**
* @struct Background
* @brief struct for backg
*/
typedef struct
{
	SDL_Surface* img;
	SDL_Rect pos;
	SDL_Surface* masque;
}backg;

/**
* @struct Personnage
* @brief struct for perso

*/
/*typedef struct
{
	SDL_Surface* img;
	SDL_Rect pos;
}perso;
*/

/**
* @struct Text
* @brief struct for text
*/
typedef struct
{
	SDL_Surface* surf;
    SDL_Rect pos;
    char txt[20];
    SDL_Color couleur;
    TTF_Font* police;
}text;

/**
* @struct Temps
* @brief struct for temps
*/
typedef struct
{
    int deb;
    int min; 
    int sec;
    text text;
}temps;

void initmap(minimap* m);
void init_background_minimap(backg* background);
// void init_perso(perso* perso);  // Legacy function - see minimap.c

void afficher_background(backg background, SDL_Surface *screen);
// void afficher_perso(perso perso, SDL_Surface *screen);  // Legacy function - see minimap.c
void afficherminimap (minimap m, SDL_Surface* screen);

SDL_Color GetPixel(SDL_Surface *backg, int x, int y);
// int collisionPP(perso p, SDL_Surface *masque);  // Legacy function - see minimap.c
void MAJminimap(int pas, int direc, minimap* m, int redim);

void minuteur(int* deb);
int charger_font(text* t, char* fich);
int init_text_temps(text* t);
int init_temps(temps* t);
void MAJ_temps(temps* t);
void afficher_temps(temps t,SDL_Surface* screen);
void liberer_text(text t);

void liberer_backg(backg b);
// void liberer_perso(perso p);  // Legacy function - see minimap.c
void liberer_minimap(minimap m);

//Tâche en Blanc

typedef struct
{
SDL_Surface *table;
SDL_Surface *bg;
SDL_Surface *t[2];
int i[3][3];
SDL_Rect r[3][3];
int turn;
int choixx;
int choixy;
}tic;

void quitgame(int * q, tic c);
void init(tic* c);
void show(tic c, SDL_Surface * screen);
void turn_played(tic* c) ;
void change(tic* c) ;
int winner_is(tic c);
void ia2(tic* c);
int game(SDL_Surface * screen);

#endif
