/* Level 1 gameplay: two players, three obstacles, one exit beacon.
 *
 * Positions in perso.pos_background are world coordinates. Nothing is
 * translated until draw time, where camera.x is subtracted. Gameplay and
 * collision work in world space; only the draw helpers know about the camera.
 */

#include "../include/game_state.h"
#include "../include/hud.h"

/* The collision box, offset inside the sprite. These numbers do not match the
 * art -- frames run 99-205px wide and 286-304px tall, so the box drifts as the
 * animation plays, and the sprite's feet sit 80px below the ground surface.
 * Slice 5 derives the box from real sprite metrics; until then this stays as
 * it was so the change is isolated to that commit. */
#define HITBOX_DX 55
#define HITBOX_DY 225
#define HITBOX_W 100
#define HITBOX_H 55
/* Sprite width the level-edge clamps assume. */
#define SPRITE_W 219

static plat_rect player_hitbox(const perso *player) {
  plat_rect hitbox;
  hitbox.x = player->pos_background.x + HITBOX_DX;
  hitbox.y = player->pos_background.y + HITBOX_DY;
  hitbox.w = HITBOX_W;
  hitbox.h = HITBOX_H;
  return hitbox;
}

static int rect_overlap(plat_rect a, plat_rect b) {
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h &&
         a.y + a.h > b.y;
}

/* Horizontal-only swept resolution against the obstacles, using the pre-move
 * x. There is no vertical response: verticality is saut() against the scalar
 * ground in ctx->posy. Slice 5 replaces both. */
static void clamp_and_resolve_obstacles(game_ctx *ctx, perso *player,
                                        int previous_x) {
  int i;
  plat_rect hitbox = player_hitbox(player);
  plat_rect previous_box = hitbox;
  previous_box.x = previous_x + HITBOX_DX;
  for (i = 0; i < LEVEL1_OBSTACLE_COUNT; ++i) {
    if (!rect_overlap(hitbox, ctx->obstacles[i])) {
      continue;
    }
    if (previous_box.x + previous_box.w <= ctx->obstacles[i].x) {
      player->pos_background.x = ctx->obstacles[i].x - HITBOX_DX - hitbox.w;
    } else if (previous_box.x >=
               ctx->obstacles[i].x + ctx->obstacles[i].w) {
      player->pos_background.x =
          ctx->obstacles[i].x + ctx->obstacles[i].w - HITBOX_DX;
    } else {
      player->pos_background.x = previous_x;
    }
    hitbox = player_hitbox(player);
  }
  if (player->pos_background.x < 0) {
    player->pos_background.x = 0;
  }
  if (player->pos_background.x > LEVEL1_W - SPRITE_W) {
    player->pos_background.x = LEVEL1_W - SPRITE_W;
  }
}

static int level1_completed(const game_ctx *ctx) {
  return rect_overlap(player_hitbox(&ctx->p), ctx->goal) ||
         rect_overlap(player_hitbox(&ctx->p1), ctx->goal);
}

static void capture_victory_stats(game_ctx *ctx) {
  ctx->victory_score_p = ctx->p.iscore / 20;
  ctx->victory_score_p1 = ctx->p1.iscore / 20;
  ctx->victory_time_sec =
      (int)((plat_ticks() - ctx->level1_start_ticks) / 1000);
  if (rect_overlap(player_hitbox(&ctx->p), ctx->goal) &&
      rect_overlap(player_hitbox(&ctx->p1), ctx->goal)) {
    ctx->victory_winner = 3;
  } else if (rect_overlap(player_hitbox(&ctx->p), ctx->goal)) {
    ctx->victory_winner = 1;
  } else if (rect_overlap(player_hitbox(&ctx->p1), ctx->goal)) {
    ctx->victory_winner = 2;
  }
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

/* Runs on every entry, including a return after ESC. The old code reset only
 * when entering from the menu, so an ESC and re-entry resumed mid-mission. */
static void level1_enter(game_ctx *ctx) {
  ctx->p.pos_background.x = 60;
  ctx->p.pos_background.y = LEVEL1_GROUND_Y;
  ctx->p.direction = 0;
  ctx->p.imag = 0;
  ctx->p.up = 0;
  ctx->p.jump = 0;
  ctx->p.acceleration = 0;
  ctx->p.iscore = 0;
  ctx->p.vie = 5;

  ctx->p1.pos_background.x = 200;
  ctx->p1.pos_background.y = LEVEL1_GROUND_Y;
  ctx->p1.direction = 0;
  ctx->p1.imag = 0;
  ctx->p1.up = 0;
  ctx->p1.jump = 0;
  ctx->p1.acceleration = 0;
  ctx->p1.iscore = 0;
  ctx->p1.vie = 5;

  ctx->posy = LEVEL1_GROUND_Y;
  ctx->posy1 = LEVEL1_GROUND_Y;
  ctx->dep = 0;
  ctx->acc = 0;
  ctx->dep1 = 0;
  ctx->acc1 = 0;
  ctx->hover = 0;
  ctx->exit_hover = 0;

  ctx->level1_start_ticks = plat_ticks();
  ctx->victory_ticks = 0;
  ctx->victory_score_p = 0;
  ctx->victory_score_p1 = 0;
  ctx->victory_time_sec = 0;
  ctx->victory_winner = 0;
  ctx->t_prev = plat_ticks();
}

static void level1_event(game_ctx *ctx, const SDL_Event *event) {
  if (event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      game_request(ctx, ST_MENU);
      break;
    /* Player 1: arrows to move, space to jump, A/D to accelerate. */
    case SDLK_UP:
      ctx->p.up = 1;
      break;
    case SDLK_SPACE:
      ypos_jump(ctx->p, &ctx->posy);
      ctx->p.jump = 1;
      ctx->p.up = 1;
      break;
    case SDLK_DOWN:
      ctx->p.pos_background.y += 20;
      if (ctx->p.pos_background.y >= LEVEL1_GROUND_Y) {
        ctx->p.pos_background.y = LEVEL1_GROUND_Y;
      }
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
    /* Player 2: K/M to move, J to jump, O up, L down. Note there is no
     * acceleration key -- acc1 is only ever cleared. Slice 5 adds parity. */
    case SDLK_o:
      ctx->p1.up = 1;
      break;
    case SDLK_j:
      ypos_jump(ctx->p1, &ctx->posy1);
      ctx->p1.jump = 1;
      ctx->p1.up = 1;
      break;
    case SDLK_l:
      ctx->p1.pos_background.y += 20;
      if (ctx->p1.pos_background.y >= LEVEL1_GROUND_Y) {
        ctx->p1.pos_background.y = LEVEL1_GROUND_Y;
      }
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
      ctx->acc = 0;
      break;
    case SDLK_a:
    case SDLK_d:
      ctx->acc = 0;
      break;
    case SDLK_k:
    case SDLK_m:
      ctx->dep1 = 0;
      ctx->p1.imag = 0;
      ctx->acc1 = 0;
      break;
    default:
      break;
    }
  }
}

/* Ramp toward the acceleration the held key implies, or decay to rest.
 * Per-frame, not per-millisecond; Slice 5 puts this on real time. */
static void apply_acceleration(perso *player, int acc_mode) {
  if (acc_mode == 1) {
    player->acceleration += 0.01;
    if (player->acceleration >= 2) {
      player->acceleration = 2;
    }
  } else if (acc_mode == 2) {
    player->acceleration -= 0.01;
    if (player->acceleration <= -2) {
      player->acceleration = -2;
    }
  } else {
    player->acceleration -= 0.05;
    if (player->acceleration <= 0) {
      player->acceleration = 0;
    }
  }
}

static void level1_update(game_ctx *ctx) {
  /* Captured before any movement, for the swept collision resolve. No event
   * handler touches x, so this matches the old capture at the loop top. */
  int prev_x_p = ctx->p.pos_background.x;
  int prev_x_p1 = ctx->p1.pos_background.x;

  if (ctx->dep == 1) {
    deplacerPerso(&ctx->p, ctx->dt);
    animerPerso(&ctx->p);
  }
  apply_acceleration(&ctx->p, ctx->acc);
  saut(&ctx->p, ctx->posy);

  if (ctx->dep1 == 1) {
    deplacerPerso(&ctx->p1, ctx->dt);
    animerPerso(&ctx->p1);
  }
  apply_acceleration(&ctx->p1, ctx->acc1);
  saut(&ctx->p1, ctx->posy1);

  clamp_and_resolve_obstacles(ctx, &ctx->p, prev_x_p);
  clamp_and_resolve_obstacles(ctx, &ctx->p1, prev_x_p1);
  update_camera(ctx);
}

static void level1_draw(game_ctx *ctx) {
  draw_scene(ctx);
  afficherPerso(&ctx->p, ctx->screen, ctx->camera.x);
  afficherPerso(&ctx->p1, ctx->screen, ctx->camera.x);
  hud_draw(ctx);

  /* Score counts rendered frames, so it reads as a frame-rate gauge. Slice 5
   * bases it on elapsed time and distance instead. */
  ctx->p.iscore++;
  ctx->p1.iscore++;

  if (level1_completed(ctx)) {
    capture_victory_stats(ctx);
    game_request(ctx, ST_VICTORY);
  }
}

const game_state_handlers level1_state = {level1_enter, level1_event,
                                          level1_update, level1_draw, NULL};
