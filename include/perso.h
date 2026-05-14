#ifndef perso_H_INCLUDED
#define perso_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <math.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

typedef struct{
SDL_Color color_score;
TTF_Font * police_score;
SDL_Surface * image[2][9];
SDL_Surface ** barre;
SDL_Surface * score; 
SDL_Rect pos_background,pos_barre,pos_score;
int direction,imag,up,jump;
double vitesse,acceleration;

char scor[20];
int iscore;
int vie;
double vect_x;
double vect_grav ;
double vect_y ; 
}perso;

void initPerso(perso *p);
void initPerso1(perso *p);
void afficherPerso(perso p, SDL_Surface * screen);
void deplacerPerso (perso *p,Uint32 dt);
void animerPerso (perso* p);
void saut(perso* p,int posy);
void ypos_jump(perso p,int *posy);
 #endif

    

