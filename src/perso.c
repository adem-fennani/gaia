#include "../include/perso.h"
#include "../include/platform.h"
#include <stdio.h>

#include <SDL/SDL_error.h>
#include <SDL/SDL_stdinc.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_video.h>
#include <stdlib.h>

static void compute_frame_anchors(const perso *p);

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
  p->wx = 60;
  p->wy = LEVEL1_GROUND_STRIP_Y - PLAYER_BOX_H;
  p->on_ground = 1;
  p->anim_acc = 0;
  perso_sync_rect(p);
  compute_frame_anchors(p);
  p->acceleration = 0;
  p->vitesse = PLAYER_WALK_SPEED;
  p->up = 0;
  p->iscore = 0;
  p->vie = 5;

  p->vect_x = 0;
  p->vect_grav = PLAYER_GRAVITY;
  p->vect_y = 0;
  p->jump = 0;
  printf("DEBUG: initPerso: Complete\n");
  fflush(stdout);
}

/* Foot anchor per frame, in surface coordinates: the horizontal centre of the
 * frame's lowest opaque band. Both players share the same art, so this is
 * computed once and indexed [direction][frame].
 *
 * Needed because the art is tightly cropped with no consistent registration
 * point -- the idle frame is 99px wide and a mid-stride frame is 205px, so
 * blitting every frame at the same x makes the character lurch sideways as it
 * walks. Anchoring on the feet keeps the contact point still. */
static int frame_anchor[2][7];
static int frame_anchor_ready = 0;

static void compute_frame_anchors(const perso *p) {
  int i, j;
  if (frame_anchor_ready) {
    return;
  }
  for (i = 0; i < 2; i++) {
    for (j = 0; j < 7; j++) {
      frame_anchor[i][j] = plat_surface_foot_anchor(p->image[i][j]);
    }
  }
  frame_anchor_ready = 1;
}

void perso_sync_rect(perso *p) {
  p->pos_background.x = (Sint16)p->wx;
  p->pos_background.y = (Sint16)p->wy;
  p->pos_background.w = PLAYER_BOX_W;
  p->pos_background.h = PLAYER_BOX_H;
}

double perso_speed(const perso *p) {
  double speed = PLAYER_WALK_SPEED + p->acceleration;
  if (speed < 0) {
    speed = 0;
  }
  return (p->direction == 1) ? -speed : speed;
}

int perso_jump(perso *p) {
  /* Only from the ground: the old code set up=1 on any press, so holding the
   * key climbed indefinitely. */
  if (!p->on_ground) {
    return 0;
  }
  p->vect_y = -PLAYER_JUMP_SPEED;
  p->on_ground = 0;
  p->up = 1;
  p->jump = 1;
  return 1;
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
  SDL_Rect dst;
  SDL_Surface *frame;
  int dir, idx;
  if (p == NULL || screen == NULL) {
    return;
  }
  dir = (p->direction == 1) ? 1 : 0;
  idx = p->imag;
  if (idx < 0 || idx > 6) {
    idx = 0;
  }
  frame = p->image[dir][idx];
  if (frame == NULL) {
    return;
  }
  /* Anchored on the feet and bottom-aligned to the collision box, so the
   * character stands on the floor instead of 80px inside it -- and stays put
   * horizontally as the frame width changes through the walk cycle. */
  dst.x = (Sint16)(p->wx + PLAYER_BOX_W / 2 - frame_anchor[dir][idx] -
                   camera_x);
  dst.y = (Sint16)(p->wy + PLAYER_BOX_H - frame->h);
  dst.w = frame->w;
  dst.h = frame->h;
  plat_blit(frame, NULL, screen, &dst);
}

void animerPerso(perso *p, double dt_seconds) {
  p->anim_acc += dt_seconds;
  while (p->anim_acc >= 1.0 / PLAYER_ANIM_FPS) {
    p->anim_acc -= 1.0 / PLAYER_ANIM_FPS;
    p->imag++;
    /* Frame 0 is the standing pose; the walk cycle is frames 1-6. */
    if (p->imag >= 7) {
      p->imag = 1;
    }
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
  p->wx = 200;
  p->wy = LEVEL1_GROUND_STRIP_Y - PLAYER_BOX_H;
  p->on_ground = 1;
  p->anim_acc = 0;
  perso_sync_rect(p);
  compute_frame_anchors(p);
  p->acceleration = 0;
  p->vitesse = PLAYER_WALK_SPEED;
  p->up = 0;
  p->iscore = 0;
  p->vie = 5;

  p->vect_x = 0;
  p->vect_grav = PLAYER_GRAVITY;
  p->vect_y = 0;
  p->jump = 0;
  printf("DEBUG: initPerso1: Complete\n");
  fflush(stdout);
}
