/* Main menu: play, settings, quit. */

#include "../include/game_state.h"

static void menu_event(game_ctx *ctx, const SDL_Event *event) {
  if (event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
    case SDLK_UP:
      ctx->hover = (ctx->hover <= 1) ? 3 : ctx->hover - 1;
      break;
    case SDLK_DOWN:
      ctx->hover = (ctx->hover >= 3) ? 1 : ctx->hover + 1;
      break;
    case SDLK_RETURN:
      if (ctx->hover == 1) {
        plat_sound_play(ctx->art.click);
        game_request(ctx, ST_LEVEL1);
      } else if (ctx->hover == 2) {
        plat_sound_play(ctx->art.click);
        game_request(ctx, ST_SETTINGS);
      } else if (ctx->hover == 3) {
        plat_sound_play(ctx->art.click);
        game_request(ctx, ST_QUIT);
      }
      break;
    default:
      break;
    }
    return;
  }

  /* Mouse position comes from event->button for both click and motion: the
   * two live in the same union and share x/y at the same offsets. */
  if (event->type == SDL_MOUSEBUTTONDOWN &&
      event->button.button == SDL_BUTTON_LEFT) {
    if (image_contains(&ctx->art.B_play, event->button.x, event->button.y)) {
      plat_sound_play(ctx->art.click);
      game_request(ctx, ST_LEVEL1);
    } else if (image_contains(&ctx->art.B_settings, event->button.x,
                              event->button.y)) {
      plat_sound_play(ctx->art.click);
      game_request(ctx, ST_SETTINGS);
    } else if (image_contains(&ctx->art.B_quit, event->button.x,
                              event->button.y)) {
      plat_sound_play(ctx->art.click);
      game_request(ctx, ST_QUIT);
    }
    return;
  }

  if (event->type == SDL_MOUSEMOTION) {
    if (image_contains(&ctx->art.B_play, event->motion.x, event->motion.y)) {
      ctx->hover = 1;
    } else if (image_contains(&ctx->art.B_settings, event->motion.x,
                              event->motion.y)) {
      ctx->hover = 2;
    } else if (image_contains(&ctx->art.B_quit, event->motion.x,
                              event->motion.y)) {
      ctx->hover = 3;
    } else {
      ctx->hover = 0;
    }
  }
}

static void menu_draw(game_ctx *ctx) {
  affichier_imag(ctx->art.background, ctx->screen);
  /* Exactly one button shows its hover variant. */
  affichier_imag(ctx->hover == 1 ? ctx->art.B_play1 : ctx->art.B_play,
                 ctx->screen);
  affichier_imag(ctx->hover == 2 ? ctx->art.B_settings1 : ctx->art.B_settings,
                 ctx->screen);
  affichier_imag(ctx->hover == 3 ? ctx->art.B_quit1 : ctx->art.B_quit,
                 ctx->screen);
}

const game_state_handlers menu_state = {NULL, menu_event, NULL, menu_draw,
                                        NULL};
