#include "../include/enemy.h"

/* Patrol segments sit in the gaps between the level's obstacles, which occupy
 * x 350-430, 850-940 and 1350-1430, with the goal at 1900-2025. */
static const double patrol[ENEMY_COUNT][2] = {
    {470, 800}, {990, 1300}, {1470, 1850}};

void enemy_reset_all(enemy *list) {
  int i;
  for (i = 0; i < ENEMY_COUNT; ++i) {
    enemy *e = &list[i];
    e->patrol_min = patrol[i][0];
    e->patrol_max = patrol[i][1];
    e->body.x = e->patrol_min;
    e->body.y = LEVEL1_GROUND_STRIP_Y - ENEMY_BOX_H;
    e->body.w = ENEMY_BOX_W;
    e->body.h = ENEMY_BOX_H;
    e->body.vy = 0;
    e->body.on_ground = 1;
    e->dir = 0;
    e->frame = 0;
    e->anim_acc = 0;
    e->alive = 1;
  }
}

void enemy_step(enemy *e, double dt, const plat_rect *solids, int solid_count,
                double ground_y, double level_w) {
  double dx;
  if (!e->alive) {
    return;
  }

  dx = (e->dir == 1) ? -ENEMY_SPEED * dt : ENEMY_SPEED * dt;

  e->body.vy += PLAYER_GRAVITY * dt;
  if (e->body.vy > PLAYER_TERMINAL_FALL) {
    e->body.vy = PLAYER_TERMINAL_FALL;
  }
  collision_move(&e->body, dx, e->body.vy * dt, solids, solid_count, ground_y,
                 level_w);

  /* Turn at the patrol bounds. Also turn if the resolver stopped us short of
   * where we asked to be, which means we walked into a solid. */
  if (e->body.x <= e->patrol_min) {
    e->body.x = e->patrol_min;
    e->dir = 0;
  } else if (e->body.x + e->body.w >= e->patrol_max) {
    e->body.x = e->patrol_max - e->body.w;
    e->dir = 1;
  }

  e->anim_acc += dt;
  while (e->anim_acc >= 1.0 / ENEMY_ANIM_FPS) {
    e->anim_acc -= 1.0 / ENEMY_ANIM_FPS;
    e->frame = (e->frame + 1) % ENEMY_FRAMES;
  }
}

static int boxes_overlap(const aabb *a, const aabb *b) {
  return a->x < b->x + b->w && a->x + a->w > b->x && a->y < b->y + b->h &&
         a->y + a->h > b->y;
}

int enemy_collide_player(enemy *list, perso *player) {
  int i;
  int damaged = 0;
  aabb pbox;

  pbox.x = player->wx;
  pbox.y = player->wy;
  pbox.w = PLAYER_BOX_W;
  pbox.h = PLAYER_BOX_H;
  pbox.vy = player->vect_y;
  pbox.on_ground = player->on_ground;

  for (i = 0; i < ENEMY_COUNT; ++i) {
    enemy *e = &list[i];
    if (!e->alive || !boxes_overlap(&pbox, &e->body)) {
      continue;
    }

    /* Falling onto the enemy's upper half kills it. Without this the enemies
     * are pure attrition with no counterplay, which is not a mission. */
    if (player->vect_y > 0 &&
        (player->wy + PLAYER_BOX_H) < (e->body.y + e->body.h * 0.5)) {
      e->alive = 0;
      player->vect_y = -PLAYER_STOMP_BOUNCE;
      player->on_ground = 0;
      continue;
    }

    if (player->invuln > 0) {
      continue;
    }

    /* Contact from any other direction costs one segment, with a grace period
     * and a knock away from the enemy. */
    if (player->vie > 0) {
      player->vie--;
    }
    player->invuln = PLAYER_INVULN_SECONDS;
    /* An impulse, not a position write: moving wx directly here could place
     * the player inside an obstacle, and the X resolver would then push them
     * out along the direction they were walking -- which sent them clean
     * through to the far side of the crate and wedged them there. */
    if (player->wx + PLAYER_BOX_W / 2.0 < e->body.x + e->body.w / 2.0) {
      player->vx_impulse = -PLAYER_KNOCKBACK_X;
    } else {
      player->vx_impulse = PLAYER_KNOCKBACK_X;
    }
    player->vect_y = -PLAYER_KNOCKBACK_Y;
    player->on_ground = 0;
    damaged = 1;
  }
  return damaged;
}
