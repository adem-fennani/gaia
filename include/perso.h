#ifndef perso_H_INCLUDED
#define perso_H_INCLUDED

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEVEL1_W 2300
/* Top of the drawn ground strip, and the surface the players stand on. */
#define LEVEL1_GROUND_STRIP_Y 515

/* The player's collision box, in world space. Deliberately independent of the
 * sprite: frames run 99-205px wide and 286-304px tall as the pose changes, so
 * a box derived from frame size would grow and shrink mid-stride. Width is
 * taken from the character's torso, height from its standing height. */
#define PLAYER_BOX_W 56
#define PLAYER_BOX_H 290

/* Movement, in pixels per second and pixels per second squared. The old code
 * used raw milliseconds with vitesse = 1, so dx was ~16px per frame from the
 * linear term alone and the 0.5*a*dt*dt term reached ~272px in a single frame
 * at the acceleration clamp -- wider than the sprite, and past whole
 * obstacles. */
#define PLAYER_WALK_SPEED 260.0
#define PLAYER_RUN_BOOST 170.0
#define PLAYER_RUN_RAMP 480.0
#define PLAYER_GRAVITY 2000.0
/* apex = v^2 / 2g = 900^2 / 4000 = ~202px, enough to clear the 130px crates. */
#define PLAYER_JUMP_SPEED 900.0
#define PLAYER_TERMINAL_FALL 1500.0
/* How fast a knockback impulse bleeds off, px/s per second. */
#define PLAYER_IMPULSE_DECAY 900.0

/* Walk cycle rate. The old animerPerso() advanced one frame per rendered
 * frame, so the cycle ran at whatever the frame rate happened to be. */
#define PLAYER_ANIM_FPS 10.0

typedef struct {
  SDL_Color color_score;
  TTF_Font *police_score;
  SDL_Surface *image[2][9];
  SDL_Surface **barre;
  SDL_Surface *score;
  SDL_Rect pos_background, pos_barre, pos_score;
  int direction, imag, up, jump;
  int on_ground;
  double vitesse, acceleration;

  /* Authoritative world position of the collision box. pos_background is
   * derived from these for drawing and for the camera; keeping the truth in
   * doubles stops the per-frame Sint16 truncation from bending the jump arc. */
  double wx, wy;
  double anim_acc; /* seconds accumulated toward the next walk frame */
  double invuln;   /* seconds of damage immunity remaining */
  /* Horizontal impulse, px/s, decaying. Used for knockback: applying it as a
   * velocity keeps it inside the collision resolver, where a direct write to
   * wx could drop the player inside a solid. */
  double vx_impulse;

  char scor[20];
  int iscore;
  int vie;
  double vect_x;
  double vect_grav;
  double vect_y;
} perso;

void initPerso(perso *p);
void initPerso1(perso *p);
void afficherPerso(const perso *p, SDL_Surface *screen, int camera_x);
/* Horizontal speed for this frame, in px/s, signed by direction. */
double perso_speed(const perso *p);
void animerPerso(perso *p, double dt_seconds);
/* Copies wx/wy into the integer pos_background used for drawing. */
void perso_sync_rect(perso *p);
void perso_jump(perso *p);
#endif
