/* Level 1 gameplay: two players, three obstacles, one exit beacon.
 *
 * Positions in perso.pos_background are world coordinates. Nothing is
 * translated until draw time, where camera.x is subtracted. Gameplay and
 * collision work in world space; only the draw helpers know about the camera.
 */

#include "../include/game_state.h"
#include "../include/collision.h"
#include "../include/enemy.h"
#include "../include/hud.h"

/* The collision box is now the player's authoritative position, so there is
 * nothing to offset: pos_background *is* the box. The old constants (+55,
 * +225, 100x55 inside a nominal 219x305 sprite) described art that does not
 * exist -- frames run 99-205px wide and 286-304px tall -- so the box drifted
 * as the animation played, and the sprite's feet sat 80px below the ground. */
static plat_rect player_hitbox(const perso *player) {
  return player->pos_background;
}

static int rect_overlap(plat_rect a, plat_rect b) {
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h &&
         a.y + a.h > b.y;
}

/* Both players, not either: this is a co-op mission, and update_camera()
 * already keeps the pair framed together. */
static int level1_completed(const game_ctx *ctx) {
  return rect_overlap(player_hitbox(&ctx->p), ctx->goal) &&
         rect_overlap(player_hitbox(&ctx->p1), ctx->goal);
}

static void capture_victory_stats(game_ctx *ctx) {
  ctx->victory_score_p = ctx->p.iscore;
  ctx->victory_score_p1 = ctx->p1.iscore;
  ctx->victory_time_sec =
      (int)((plat_ticks() - ctx->level1_start_ticks) / 1000);
  /* Completion requires both, so this is always a team victory. The other
   * cases stay reachable for a future single-player mission. */
  ctx->victory_winner = 3;
  ctx->victory_ticks = plat_ticks();
}

/* Centre on the midpoint of both players, clamped to the level. */
static void update_camera(game_ctx *ctx) {
  int mid_x = (ctx->p.pos_background.x + ctx->p1.pos_background.x) / 2;
  ctx->camera.x = mid_x - SCREEN_W / 2;
  if (ctx->camera.x < 0) {
    ctx->camera.x = 0;
  }
  if (ctx->camera.x > LEVEL1_W - SCREEN_W) {
    ctx->camera.x = LEVEL1_W - SCREEN_W;
  }
  ctx->camera.y = 0;
  ctx->camera.w = SCREEN_W;
  ctx->camera.h = SCREEN_H;
}

/* Scroll one backdrop layer at rate num/den of the camera. The clamps are a
 * backstop: the layers are generated at exactly the size these rates need, so
 * if one engages, verify_assets.py should have failed the build first. */
static void draw_parallax_layer(game_ctx *ctx, const image *layer, int num,
                                int den) {
  plat_rect src, dst;
  if (layer->img == NULL) {
    return;
  }
  src.x = ctx->camera.x * num / den;
  src.y = 0;
  src.w = SCREEN_W;
  src.h = SCREEN_H;
  if (src.x > layer->img->w - SCREEN_W) {
    src.x = layer->img->w - SCREEN_W;
  }
  if (src.x < 0) {
    src.x = 0;
  }
  dst.x = 0;
  dst.y = 0;
  dst.w = SCREEN_W;
  dst.h = SCREEN_H;
  plat_blit(layer->img, &src, ctx->screen, &dst);
}

static void draw_world_sprite(game_ctx *ctx, const image *sprite, int x,
                              int y) {
  plat_rect dst;
  if (sprite->img == NULL) {
    return;
  }
  dst.x = x - ctx->camera.x;
  dst.y = y;
  dst.w = sprite->img->w;
  dst.h = sprite->img->h;
  plat_blit(sprite->img, NULL, ctx->screen, &dst);
}

static void draw_scene(game_ctx *ctx) {
  int i;
  int x;

  draw_parallax_layer(ctx, &ctx->art.backgame, PARALLAX_FAR_NUM,
                      PARALLAX_FAR_DEN);
  draw_parallax_layer(ctx, &ctx->art.backgame_near, PARALLAX_NEAR_NUM,
                      PARALLAX_NEAR_DEN);

  if (ctx->art.ground_tile.img != NULL) {
    /* Tile from the first boundary left of the viewport, so the strip scrolls
     * with the camera instead of sliding under it. */
    for (x = -(ctx->camera.x % GROUND_TILE_W); x < SCREEN_W;
         x += GROUND_TILE_W) {
      plat_rect dst;
      dst.x = x;
      dst.y = LEVEL1_GROUND_STRIP_Y;
      dst.w = ctx->art.ground_tile.img->w;
      dst.h = ctx->art.ground_tile.img->h;
      plat_blit(ctx->art.ground_tile.img, NULL, ctx->screen, &dst);
    }
  } else {
    plat_rect ground;
    ground.x = -ctx->camera.x;
    ground.y = LEVEL1_GROUND_STRIP_Y;
    ground.w = LEVEL1_W;
    ground.h = SCREEN_H - ground.y;
    plat_fill_rect(ctx->screen, &ground, 79, 19, 37);
  }

  for (i = 0; i < LEVEL1_OBSTACLE_COUNT; ++i) {
    const image *art = (i == 1) ? &ctx->art.obs_crate_tall : &ctx->art.obs_crate;
    draw_world_sprite(ctx, art, ctx->obstacles[i].x, ctx->obstacles[i].y);
  }
  draw_world_sprite(ctx, &ctx->art.goal_beacon, ctx->goal.x, ctx->goal.y);

}

/* Positions a player at its spawn, standing on the ground. */
static void spawn(perso *player, int x) {
  player->wx = x;
  player->wy = LEVEL1_GROUND_STRIP_Y - PLAYER_BOX_H;
  player->vect_y = 0;
  player->on_ground = 1;
  player->direction = 0;
  player->imag = 0;
  player->anim_acc = 0;
  player->up = 0;
  player->jump = 0;
  player->acceleration = 0;
  player->iscore = 0;
  player->vie = 5;
  player->invuln = 0;
  player->vx_impulse = 0;
  perso_sync_rect(player);
}

/* Runs on every entry, including a return after ESC. The old code reset only
 * when entering from the menu, so an ESC and re-entry resumed mid-mission. */
static void level1_enter(game_ctx *ctx) {
  spawn(&ctx->p, 60);
  spawn(&ctx->p1, 200);
  enemy_reset_all(ctx->enemies);

  ctx->dep = 0;
  ctx->acc = 0;
  ctx->dep1 = 0;
  ctx->acc1 = 0;
  ctx->hover = 0;
  ctx->exit_hover = 0;
  ctx->step_acc = 0;

  ctx->level1_start_ticks = plat_ticks();
  ctx->victory_ticks = 0;
  ctx->victory_score_p = 0;
  ctx->victory_score_p1 = 0;
  ctx->victory_time_sec = 0;
  ctx->victory_winner = 0;
  ctx->mission_failed = 0;
  ctx->t_prev = plat_ticks();
}

static void level1_event(game_ctx *ctx, const SDL_Event *event) {
  if (event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      game_request(ctx, ST_MENU);
      break;
    /* Player 1: arrows to move, space to jump, A/D to run. UP also jumps --
     * it used to set up=1 directly, which started a fall from midair without
     * ever giving upward velocity. */
    case SDLK_UP:
      perso_jump(&ctx->p);
      break;
    case SDLK_SPACE:
      perso_jump(&ctx->p);
      break;
    case SDLK_LEFT:
      ctx->p.direction = 1;
      ctx->dep = 1;
      break;
    case SDLK_RIGHT:
      ctx->p.direction = 0;
      ctx->dep = 1;
      break;
    case SDLK_a:
      ctx->acc = 1;
      break;
    case SDLK_d:
      ctx->acc = 2;
      break;
    /* Player 2: K/M to move, J or O to jump, W/X to run. acc1 previously had
     * no key at all -- it was only ever cleared -- so player 2 could not run. */
    case SDLK_o:
      perso_jump(&ctx->p1);
      break;
    case SDLK_j:
      perso_jump(&ctx->p1);
      break;
    case SDLK_w:
      ctx->acc1 = 1;
      break;
    case SDLK_x:
      ctx->acc1 = 2;
      break;
    case SDLK_k:
      ctx->p1.direction = 1;
      ctx->dep1 = 1;
      break;
    case SDLK_m:
      ctx->p1.direction = 0;
      ctx->dep1 = 1;
      break;
    default:
      break;
    }
    return;
  }

  if (event->type == SDL_KEYUP) {
    switch (event->key.keysym.sym) {
    case SDLK_LEFT:
    case SDLK_RIGHT:
      ctx->dep = 0;
      ctx->p.imag = 0;
      break;
    case SDLK_a:
    case SDLK_d:
      ctx->acc = 0;
      break;
    case SDLK_k:
    case SDLK_m:
      ctx->dep1 = 0;
      ctx->p1.imag = 0;
      break;
    case SDLK_w:
    case SDLK_x:
      ctx->acc1 = 0;
      break;
    default:
      break;
    }
  }
}

/* Ramp the run boost toward what the held key implies, or decay to a walk.
 * Now per-second rather than per-frame: the old +-0.01 and -0.05 steps were
 * applied once per rendered frame, so how fast you accelerated depended on
 * the frame rate. */
static void apply_acceleration(perso *player, int acc_mode, double dt) {
  double target = 0;
  if (acc_mode == 1) {
    target = PLAYER_RUN_BOOST;
  } else if (acc_mode == 2) {
    target = -PLAYER_RUN_BOOST;
  }
  if (player->acceleration < target) {
    player->acceleration += PLAYER_RUN_RAMP * dt;
    if (player->acceleration > target) {
      player->acceleration = target;
    }
  } else if (player->acceleration > target) {
    player->acceleration -= PLAYER_RUN_RAMP * dt;
    if (player->acceleration < target) {
      player->acceleration = target;
    }
  }
}

/* Advances one player by exactly `dt` seconds. */
static void step_player(game_ctx *ctx, perso *player, int moving, int acc_mode,
                        double dt) {
  aabb body;
  double dx = 0;

  apply_acceleration(player, acc_mode, dt);

  if (player->invuln > 0) {
    player->invuln -= dt;
    if (player->invuln < 0) {
      player->invuln = 0;
    }
  }

  if (moving) {
    dx = perso_speed(player) * dt;
    animerPerso(player, dt);
  }

  /* Knockback rides along with normal movement and bleeds off. */
  dx += player->vx_impulse * dt;
  if (player->vx_impulse > 0) {
    player->vx_impulse -= PLAYER_IMPULSE_DECAY * dt;
    if (player->vx_impulse < 0) {
      player->vx_impulse = 0;
    }
  } else if (player->vx_impulse < 0) {
    player->vx_impulse += PLAYER_IMPULSE_DECAY * dt;
    if (player->vx_impulse > 0) {
      player->vx_impulse = 0;
    }
  }

  /* Gravity, with a terminal velocity so a long fall cannot tunnel through a
   * solid in a single step. */
  player->vect_y += PLAYER_GRAVITY * dt;
  if (player->vect_y > PLAYER_TERMINAL_FALL) {
    player->vect_y = PLAYER_TERMINAL_FALL;
  }

  body.x = player->wx;
  body.y = player->wy;
  body.w = PLAYER_BOX_W;
  body.h = PLAYER_BOX_H;
  body.vy = player->vect_y;
  body.on_ground = player->on_ground;

  collision_move(&body, dx, player->vect_y * dt, ctx->obstacles,
                 LEVEL1_OBSTACLE_COUNT, LEVEL1_GROUND_STRIP_Y, LEVEL1_W);

  player->wx = body.x;
  player->wy = body.y;
  player->vect_y = body.vy;
  player->on_ground = body.on_ground;
  if (player->on_ground) {
    player->up = 0;
    player->jump = 0;
  }
  perso_sync_rect(player);
}

static void level1_update(game_ctx *ctx) {
  /* Fixed timestep. Physics runs in constant LEVEL1_STEP slices regardless of
   * frame rate, so behaviour no longer depends on how fast the machine draws.
   * The accumulator is capped so a stall cannot queue up a huge catch-up burst
   * (the "spiral of death"), and dt itself is capped upstream in game.c. */
  double frame = ctx->dt / 1000.0;
  if (frame > LEVEL1_MAX_FRAME) {
    frame = LEVEL1_MAX_FRAME;
  }
  ctx->step_acc += frame;
  if (ctx->step_acc > LEVEL1_MAX_FRAME) {
    ctx->step_acc = LEVEL1_MAX_FRAME;
  }

  while (ctx->step_acc >= LEVEL1_STEP) {
    ctx->step_acc -= LEVEL1_STEP;
    int i;
    step_player(ctx, &ctx->p, ctx->dep == 1, ctx->acc, LEVEL1_STEP);
    step_player(ctx, &ctx->p1, ctx->dep1 == 1, ctx->acc1, LEVEL1_STEP);

    for (i = 0; i < ENEMY_COUNT; ++i) {
      enemy_step(&ctx->enemies[i], LEVEL1_STEP, ctx->obstacles,
                 LEVEL1_OBSTACLE_COUNT, LEVEL1_GROUND_STRIP_Y, LEVEL1_W);
    }
    /* Contact is resolved after both sides have moved, so a hit cannot depend
     * on which of them stepped first. */
    enemy_collide_player(ctx->enemies, &ctx->p);
    enemy_collide_player(ctx->enemies, &ctx->p1);
    perso_sync_rect(&ctx->p);
    perso_sync_rect(&ctx->p1);
  }

  update_camera(ctx);

  /* Score is progress plus a time bonus, not a count of rendered frames.
   * iscore++ per frame made the score a frame-rate readout. */
  ctx->p.iscore = (int)(ctx->p.wx / 10);
  ctx->p1.iscore = (int)(ctx->p1.wx / 10);

  /* Either player running out of health fails the mission: it is co-op, and
   * completion already requires both of them at the beacon. */
  if (ctx->p.vie <= 0 || ctx->p1.vie <= 0) {
    ctx->mission_failed = 1;
    capture_victory_stats(ctx);
    game_request(ctx, ST_VICTORY);
    return;
  }

  if (level1_completed(ctx)) {
    capture_victory_stats(ctx);
    game_request(ctx, ST_VICTORY);
  }
}

static void draw_enemies(game_ctx *ctx) {
  int i;
  for (i = 0; i < ENEMY_COUNT; ++i) {
    const enemy *e = &ctx->enemies[i];
    plat_surface *frame;
    plat_rect dst;
    if (!e->alive) {
      continue;
    }
    frame = ctx->art.enemy_frames[e->dir][e->frame % ENEMY_FRAMES];
    if (frame == NULL) {
      continue;
    }
    /* Bottom-aligned and centred on the collision box, the same convention
     * the players use, so art and collider cannot drift apart. */
    dst.x = (Sint16)(e->body.x + (ENEMY_BOX_W - frame->w) / 2 - ctx->camera.x);
    dst.y = (Sint16)(e->body.y + ENEMY_BOX_H - frame->h);
    dst.w = frame->w;
    dst.h = frame->h;
    plat_blit(frame, NULL, ctx->screen, &dst);
  }
}

/* Blink while immune, so the grace period is visible rather than just felt. */
static int player_visible(const perso *player) {
  if (player->invuln <= 0) {
    return 1;
  }
  return ((int)(player->invuln * 12.0)) % 2 == 0;
}

static void level1_draw(game_ctx *ctx) {
  draw_scene(ctx);
  draw_enemies(ctx);
  if (player_visible(&ctx->p)) {
    afficherPerso(&ctx->p, ctx->screen, ctx->camera.x);
  }
  if (player_visible(&ctx->p1)) {
    afficherPerso(&ctx->p1, ctx->screen, ctx->camera.x);
  }
  hud_draw(ctx);
}

const game_state_handlers level1_state = {level1_enter, level1_event,
                                          level1_update, level1_draw, NULL};
