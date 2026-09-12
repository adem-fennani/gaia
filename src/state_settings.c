/* Settings screen: music volume and a way back to the menu. */

#include "../include/audio.h"
#include "../include/game_state.h"

/* Travel limits for the slider knob, in screen coordinates. */
#define SLIDER_MIN_X 515
#define SLIDER_MAX_X 720

/* The knob's position is derived from the volume rather than nudged alongside
 * it. Previously the two moved independently -- the knob by a flat +-15px per
 * press, the volume by +-9 or +-10 -- so they desynced immediately, and the
 * knob started pinned at the far left while the volume sat at its 64 default,
 * i.e. showing empty at half volume. Deriving it makes that impossible. */
void settings_sync_slider(game_ctx *ctx) {
  int span = SLIDER_MAX_X - SLIDER_MIN_X;
  int v = ctx->volume;
  if (v < 0) {
    v = 0;
  }
  if (v > AUDIO_VOLUME_MAX) {
    v = AUDIO_VOLUME_MAX;
  }
  ctx->art.slayed.pos.x = (Sint16)(SLIDER_MIN_X + v * span / AUDIO_VOLUME_MAX);
}

static void slider_nudge(game_ctx *ctx, int volume_delta) {
  /* audio_music_volume clamps and returns what it applied, so ctx->volume
   * cannot drift outside the mixer's range. The old code added +-9 or +-10
   * without any bound at all. */
  ctx->volume = audio_music_volume(ctx->volume + volume_delta);
  settings_sync_slider(ctx);
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
      audio_play(AUDIO_MENU_CLICK);
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
