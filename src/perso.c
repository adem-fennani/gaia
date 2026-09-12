#include "../include/perso.h"
#include "../include/platform.h"
#include <stdio.h>

#include <SDL/SDL_error.h>
#include <SDL/SDL_stdinc.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_video.h>
#include <stdlib.h>

void initPerso(perso *p) {
  int i, j;
  char pers[50];
  printf("DEBUG: initPerso: Starting...\n");
  fflush(stdout);
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 7; j++) {
      sprintf(pers, "assets/img/perso/image%d-%d.png", i, j);
      p->image[i][j] = plat_image_load(pers);
      if (p->image[i][j] == NULL) {
        printf("Error loading perso surface %d-%d: %s\n", i, j,
               SDL_GetError());
      }
    }
  }
  printf("DEBUG: initPerso: Character frames created\n");
  fflush(stdout);

  p->barre = (SDL_Surface **)malloc(sizeof(SDL_Surface *) * 6);
  if (p->barre == NULL) {
    printf("Error: Memory allocation failed for barre\n");
    return;
  }
  for (i = 0; i < 6; i++) {
    sprintf(pers, "assets/img/hud/barre/barre_%d.png", i);
    p->barre[i] = plat_image_load(pers);
    if (p->barre[i] == NULL) {
      printf("Error loading barre surface %d: %s\n", i, SDL_GetError());
    }
  }
  printf("DEBUG: initPerso: Health bars created\n");
  fflush(stdout);
  p->police_score = plat_font_open("assets/fonts/Raimen.ttf", 40);
  if (p->police_score == NULL) {
    printf("Error loading font Raimen.ttf: %s\n", TTF_GetError());
  }
  p->color_score.r = 0;
  p->color_score.g = 10;
  p->color_score.b = 0;
  p->pos_score.x = 1;
  p->pos_score.y = 70;
  p->pos_barre.x = 0;
  p->pos_barre.y = 0;
  p->direction = 0;
  p->imag = 0;
  p->pos_background.x = 60;
  p->pos_background.y = 300;
  p->pos_background.w = 219;
  p->pos_background.h = 305;
  p->acceleration = 0;
  p->vitesse = 1;
  p->up = 0;
  p->iscore = 0;
  p->vie = 5;

  p->vect_x = 5;
  p->vect_grav = 0.4;
  p->vect_y = -6.5;
  p->jump = 0;
  printf("DEBUG: initPerso: Complete\n");
  fflush(stdout);
}

/* Draws the character only. The health bar and score moved to src/hud.c,
 * which owns HUD layout.
 *
 * Taking perso by value is what made the old version leak: it rendered the
 * score with TTF_RenderText_Solid and assigned the result to p.score on the
 * caller's *copy*, so the surface's only pointer died on return -- two
 * surfaces per frame, forever, while cleanup_game() dutifully freed the
 * always-NULL originals. A const pointer makes that class of mistake
 * impossible. perso.score and perso.scor are now unused. */
void afficherPerso(const perso *p, SDL_Surface *screen, int camera_x) {
  SDL_Rect screen_pos;
  if (p == NULL || screen == NULL) {
    return;
  }
  screen_pos = p->pos_background;
  screen_pos.x -= camera_x;
  if (p->image[p->direction][p->imag] != NULL) {
    plat_blit(p->image[p->direction][p->imag], NULL, screen, &screen_pos);
  }
}

void animerPerso(perso *p) {
  p->imag++;
  if (p->imag >= 7)
    p->imag = 1;
}

void deplacerPerso(perso *p, Uint32 dt) {
  double dx;
  dx = 0.5 * p->acceleration * dt * dt + p->vitesse * dt;

  switch (p->direction) {

  case 0:
    p->pos_background.x += dx;
    if (p->pos_background.x >= LEVEL1_W - 219) {
      p->pos_background.x = LEVEL1_W - 219;
    }
    break;

  case 1:
    p->pos_background.x -= dx;
    if (p->pos_background.x <= 0) {
      p->pos_background.x = 0;
    }
    break;

  default:
    break;
  }
}

void saut(perso *p, int posy) {
  if (p->up == 1) {
    if (p->jump == 1) {
      switch (p->direction) {
      case 0:
        p->pos_background.x += p->vect_x;
        break;
      case 1:
        p->pos_background.x -= p->vect_x;
        break;
      default:
        break;
      }
    }
    p->pos_background.y += p->vect_y;
    p->vect_y += p->vect_grav;
  }
  if (p->pos_background.y > posy) {
    p->vect_y = -6.5;
    p->up = 0;
    p->jump = 0;
    p->pos_background.y = posy;
  }

  if (p->pos_background.x <= 0) {
    p->pos_background.x = 0;
  }
  if (p->pos_background.x >= LEVEL1_W - 219) {
    p->pos_background.x = LEVEL1_W - 219;
  }
}

void ypos_jump(perso p, int *posy) {
  if (p.up == 0) {
    *posy = p.pos_background.y;
  }
}

void initPerso1(perso *p) {
  int i, j;
  char pers[50];
  printf("DEBUG: initPerso1: Starting...\n");
  fflush(stdout);
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 7; j++) {
      sprintf(pers, "assets/img/perso/image%d-%d.png", i, j);
      p->image[i][j] = plat_image_load(pers);
      if (p->image[i][j] == NULL) {
        printf("Error loading perso surface for player 2 %d-%d: %s\n", i, j,
               SDL_GetError());
      }
    }
  }
  printf("DEBUG: initPerso1: Character frames created\n");
  fflush(stdout);

  p->barre = (SDL_Surface **)malloc(sizeof(SDL_Surface *) * 6);
  if (p->barre == NULL) {
    printf("Error: Memory allocation failed for barre\n");
    return;
  }
  for (i = 0; i < 6; i++) {
    sprintf(pers, "assets/img/hud/barre1/barre_%d.png", i);
    p->barre[i] = plat_image_load(pers);
    if (p->barre[i] == NULL) {
      printf("Error loading barre1 surface %d: %s\n", i, SDL_GetError());
    }
  }
  printf("DEBUG: initPerso1: Health bars created\n");
  fflush(stdout);
  p->police_score = plat_font_open("assets/fonts/Raimen.ttf", 40);
  if (p->police_score == NULL) {
    printf("Error loading font Raimen.ttf: %s\n", TTF_GetError());
  }
  p->color_score.r = 0;
  p->color_score.g = 10;
  p->color_score.b = 0;
  p->pos_score.x = 575;
  p->pos_score.y = 70;
  p->pos_barre.x = 575;
  p->pos_barre.y = 0;
  p->direction = 0;
  p->imag = 0;
  p->pos_background.x = 200;
  p->pos_background.y = 300;
  p->pos_background.w = 219;
  p->pos_background.h = 305;
  p->acceleration = 0;
  p->vitesse = 1;
  p->up = 0;
  p->iscore = 0;
  p->vie = 5;

  p->vect_x = 5;
  p->vect_grav = 0.4;
  p->vect_y = -6.5;
  p->jump = 0;
  printf("DEBUG: initPerso1: Complete\n");
  fflush(stdout);
}
