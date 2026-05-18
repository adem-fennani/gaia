#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  SDL_Surface *img;
  SDL_Rect pos;
} image;

// ken enty fil menu

void init_background(image *background, char *nom);
void init_bouton(image *bouton1, char *nom1, image *bouton2, char *nom2,
                 image *bouton3, char *nom3);

// ken enty fil setting
void init_volume_slayed(image *bouton_slide, char *nom);
void init_retour_bouton(image *retour, char *nom);
// void init_boutons(image *retour,char *nom);

//
void affichier_imag(image p, SDL_Surface *screen);

void librer(image p);

#endif
