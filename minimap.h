#ifndef MINIMAP_H_INCLUDED
#define MINIMAP_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

typedef struct
{
	SDL_Surface* img;
	SDL_Rect pos;
	SDL_Surface* perso_img;
	SDL_Rect perso_pos;
}minimap;

typedef struct
{
	SDL_Surface* img;
	SDL_Rect pos;
	SDL_Surface* masque;
}backg;

typedef struct
{
	SDL_Surface* img;
	SDL_Rect pos;
}perso;

void initmap(minimap* m);
void init_background(backg* background);
void init_perso(perso* perso);
void afficher_background(backg background, SDL_Surface *screen);
void afficher_perso(perso perso, SDL_Surface *screen);
void afficherminimap (minimap m, SDL_Surface* screen);
SDL_Color GetPixel(SDL_Surface *backg, int x, int y);
int collisionPP(perso p, SDL_Surface *masque);
void MAJminimap(int pas, int direc, minimap* m, int redim);
void liberer_backg(backg b);
void liberer_perso(perso p);
void liberer_minimap(minimap m);
#endif
