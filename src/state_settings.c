/* Settings screen: music volume and a way back to the menu. */

#include "../include/game_state.h"

/* Travel limits for the slider knob, in screen coordinates. */
#define SLIDER_MIN_X 515
#define SLIDER_MAX_X 720
#define SLIDER_STEP 15

static void slider_nudge(game_ctx *ctx, int volume_delta) {
  ctx->volume += volume_delta;
  /* plat_music_volume clamps to the mixer's range; ctx->volume itself is
   * still unbounded, which Slice 7 fixes along with the rest of the audio. */
  plat_music_volume(ctx->volume);
  ctx->art.slayed.pos.x += (volume_delta > 0) ? SLIDER_STEP : -SLIDER_STEP;
  if (ctx->art.slayed.pos.x <= SLIDER_MIN_X) {
    ctx->art.slayed.pos.x = SLIDER_MIN_X;
  }
  if (ctx->art.slayed.pos.x >= SLIDER_MAX_X) {
    ctx->art.slayed.pos.x = SLIDER_MAX_X;
  }
}

static int rect_contains(const plat_rect *rect, int x, int y) {
  return x > rect->x && x < rect->x + rect->w && y > rect->y &&
         y < rect->y + rect->h;
}

static void settings_event(game_ctx *ctx, const SDL_Event *event) {
  if (event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
    case SDLK_ESCAPE:
      game_request(ctx, ST_MENU);
      break;
    case SDLK_LEFT:
      slider_nudge(ctx, -9);
      break;
    case SDLK_RIGHT:
      slider_nudge(ctx, 9);
      break;
    default:
      break;
    }
    return;
  }

  if (event->type == SDL_MOUSEBUTTONDOWN &&
      event->button.button == SDL_BUTTON_LEFT) {
    if (rect_contains(&ctx->pos_plus, event->button.x, event->button.y)) {
      slider_nudge(ctx, 10);
    } else if (rect_contains(&ctx->pos_moin, event->button.x,
                             event->button.y)) {
      slider_nudge(ctx, -10);
    } else if (image_contains(&ctx->art.exits, event->button.x,
                              event->button.y)) {
      plat_sound_play(ctx->art.click);
      game_request(ctx, ST_MENU);
    }
    return;
  }

  if (event->type == SDL_MOUSEMOTION) {
    ctx->exit_hover =
        image_contains(&ctx->art.exits, event->motion.x, event->motion.y);
  }
}

static void settings_draw(game_ctx *ctx) {
  affichier_imag(ctx->art.settings, ctx->screen);
  affichier_imag(ctx->art.slayed, ctx->screen);
  affichier_imag(ctx->exit_hover ? ctx->art.exits1 : ctx->art.exits,
                 ctx->screen);
}

const game_state_handlers settings_state = {NULL, settings_event, NULL,
                                            settings_draw, NULL};
