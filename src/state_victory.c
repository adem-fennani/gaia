/* Mission complete screen. Returns to the menu on input, or after a timeout. */

#include "../include/game_state.h"
#include <stdio.h>

static void victory_event(game_ctx *ctx, const SDL_Event *event) {
  if (event->type == SDL_KEYDOWN) {
    if (event->key.keysym.sym == SDLK_RETURN ||
        event->key.keysym.sym == SDLK_ESCAPE ||
        event->key.keysym.sym == SDLK_SPACE) {
      game_request(ctx, ST_MENU);
    }
    return;
  }
  if (event->type == SDL_MOUSEBUTTONDOWN &&
      event->button.button == SDL_BUTTON_LEFT) {
    game_request(ctx, ST_MENU);
  }
}

static void victory_update(game_ctx *ctx) {
  if ((Uint32)(plat_ticks() - ctx->victory_ticks) >=
      VICTORY_AUTO_RETURN_MS) {
    game_request(ctx, ST_MENU);
  }
}

static void victory_draw(game_ctx *ctx) {
  plat_rect panel;
  plat_color title = {255, 240, 90, 0};
  /* Failure gets a colder panel, so the two outcomes are distinguishable at a
   * glance and not only by reading the heading. */
  plat_color white = {255, 255, 255, 0};
  plat_font *font = ctx->p.police_score;
  char line[128];

  panel.x = 0;
  panel.y = 0;
  panel.w = SCREEN_W;
  panel.h = SCREEN_H;
  if (ctx->mission_failed) {
    plat_fill_rect(ctx->screen, &panel, 42, 16, 22);
  } else {
    plat_fill_rect(ctx->screen, &panel, 18, 20, 42);
  }

  if (ctx->mission_failed) {
    plat_color red = {255, 120, 110, 0};
    plat_text_draw(ctx->screen, font, "MISSION FAILED", 440, 120, red);
    plat_text_draw(ctx->screen, font,
                   "A squad member ran out of health.", 355, 205, white);
  } else {
    plat_text_draw(ctx->screen, font, "MISSION COMPLETE", 420, 120, title);
  }
  if (ctx->mission_failed) {
    /* Fall through to the score lines below. */
  } else if (ctx->victory_winner == 3) {
    plat_text_draw(ctx->screen, font,
                   "Team victory: both players reached the exit beacon.", 280,
                   205, white);
  } else if (ctx->victory_winner == 1) {
    plat_text_draw(ctx->screen, font, "Player 1 reached the exit beacon.", 360,
                   205, white);
  } else if (ctx->victory_winner == 2) {
    plat_text_draw(ctx->screen, font, "Player 2 reached the exit beacon.", 360,
                   205, white);
  } else {
    plat_text_draw(ctx->screen, font, "Mission complete.", 450, 205, white);
  }

  snprintf(line, sizeof(line), "Player 1 score: %d", ctx->victory_score_p);
  plat_text_draw(ctx->screen, font, line, 395, 280, white);
  snprintf(line, sizeof(line), "Player 2 score: %d", ctx->victory_score_p1);
  plat_text_draw(ctx->screen, font, line, 395, 320, white);
  snprintf(line, sizeof(line), "Completion time: %d seconds",
           ctx->victory_time_sec);
  plat_text_draw(ctx->screen, font, line, 380, 360, white);
  plat_text_draw(ctx->screen, font,
                 "Press ENTER, ESC, SPACE, or click to return to the menu",
                 220, 440, ctx->mission_failed ? white : title);
}

const game_state_handlers victory_state = {NULL, victory_event, victory_update,
                                           victory_draw, NULL};
