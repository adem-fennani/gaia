/* Application entry point: owns the game context, the main loop, and state
 * transitions. See include/game_state.h for the state machine's shape.
 *
 * Lifecycle is fixed: init_engine -> load_game_resources -> run_game_loop ->
 * cleanup_game. Every resource for every state is still loaded up front and
 * freed in one place, so adding a state or an asset means editing both
 * load_game_resources() and cleanup_game().
 */

#include "../include/game_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Named g_game rather than `game` because include/minimap.h still declares a
 * `void game(...)` -- the unreachable tic-tac-toe that Slice 8 deletes. */
static game_ctx g_game;

static const game_state_handlers *handlers_for(game_state state) {
  switch (state) {
  case ST_MENU:
    return &menu_state;
  case ST_LEVEL1:
    return &level1_state;
  case ST_SETTINGS:
    return &settings_state;
  case ST_VICTORY:
    return &victory_state;
  case ST_QUIT:
  default:
    return NULL;
  }
}

void game_request(game_ctx *ctx, game_state next) { ctx->next = next; }

int image_contains(const image *img, int x, int y) {
  if (img->img == NULL) {
    return 0;
  }
  return x > img->pos.x && x < img->pos.x + img->img->w && y > img->pos.y &&
         y < img->pos.y + img->img->h;
}

bool init_engine(void) {
  memset(&g_game, 0, sizeof(g_game));
  if (!plat_init(SCREEN_W, SCREEN_H, "GAIA REPRESENT")) {
    return false;
  }
  g_game.screen = plat_screen();
  return true;
}

void load_game_resources(void) {
  game_assets *art = &g_game.art;

  /* Level 1 scene art. The backdrop layers are built by
   * scripts/gen_placeholder_assets.py from Niv1.png, extended to the full
   * screen height: Niv1.png is 2048x341, so blitting a SCREEN_H-tall source
   * rect straight out of it left the bottom 309 rows unwritten. */
  init_background(&art->backgame, "assets/img/levels/niv1_far.png");
  art->backgame.pos.x = 0;
  art->backgame.pos.y = 0;
  art->backgame.pos.w = SCREEN_W;
  art->backgame.pos.h = SCREEN_H;
  init_background(&art->backgame_near, "assets/img/levels/niv1_near.png");
  art->obs_crate.img = plat_image_load("assets/img/level1/crate.png");
  art->obs_crate_tall.img =
      plat_image_load("assets/img/level1/crate_tall.png");
  art->goal_beacon.img = plat_image_load("assets/img/level1/goal_beacon.png");
  art->ground_tile.img = plat_image_load("assets/img/level1/ground_tile.png");

  art->click = plat_sound_load("assets/audio/mouseclick.wav");
  if (art->click != NULL) {
    plat_sound_volume(art->click, plat_music_max_volume() / 3);
  }

  init_background(&art->background, "assets/img/background.jpg");
  init_bouton(&art->B_play, "assets/img/B_play.png", &art->B_settings,
              "assets/img/B_settings.png", &art->B_quit,
              "assets/img/B_quit.png");
  init_bouton(&art->B_play1, "assets/img/B_play1.png", &art->B_settings1,
              "assets/img/B_settings1.png", &art->B_quit1,
              "assets/img/B_quit1.png");
  init_background(&art->settings, "assets/img/settings.png");
  init_retour_bouton(&art->exits, "assets/img/exits.png");
  init_retour_bouton(&art->exits1, "assets/img/exits1.png");
  init_volume_slayed(&art->slayed, "assets/img/slayed.png");

  /* Volume +/- click targets on the settings screen. */
  g_game.pos_moin.x = 489;
  g_game.pos_moin.y = 227;
  g_game.pos_moin.w = 512 - 489;
  g_game.pos_moin.h = 235 - 227;
  g_game.pos_plus.x = 730;
  g_game.pos_plus.y = 214;
  g_game.pos_plus.w = 30;
  g_game.pos_plus.h = 34;

  initPerso(&g_game.p);
  initPerso1(&g_game.p1);
  initmap(&g_game.map);
  init_temps(&g_game.clock);

  /* Level 1 geometry, hardcoded here. A real level format is out of scope. */
  g_game.obstacles[0].x = 350;
  g_game.obstacles[0].y = 525;
  g_game.obstacles[0].w = 80;
  g_game.obstacles[0].h = 125;
  g_game.obstacles[1].x = 850;
  g_game.obstacles[1].y = 520;
  g_game.obstacles[1].w = 90;
  g_game.obstacles[1].h = 130;
  g_game.obstacles[2].x = 1350;
  g_game.obstacles[2].y = 525;
  g_game.obstacles[2].w = 80;
  g_game.obstacles[2].h = 125;
  g_game.goal.x = 1900;
  g_game.goal.y = 360;
  g_game.goal.w = 125;
  g_game.goal.h = 180;

  g_game.running = 1;
  g_game.state = ST_MENU;
  g_game.next = ST_MENU;
  g_game.hover = 0;
  g_game.exit_hover = 0;
  g_game.volume = 64;
  g_game.t_prev = plat_ticks();
}

void run_game_loop(void) {
  SDL_Event event;

  while (g_game.running) {
    const game_state_handlers *handlers = handlers_for(g_game.state);

    g_game.dt = plat_ticks() - g_game.t_prev;
    g_game.t_prev = plat_ticks();

    while (plat_event_poll(&event)) {
      /* Closing the window ends the program from any state, so it is handled
       * once here rather than repeated in all four handlers. */
      if (event.type == SDL_QUIT) {
        g_game.running = 0;
        break;
      }
      if (handlers != NULL && handlers->on_event != NULL) {
        handlers->on_event(&g_game, &event);
      }
    }

    if (handlers != NULL && handlers->on_update != NULL) {
      handlers->on_update(&g_game);
    }
    if (handlers != NULL && handlers->on_draw != NULL) {
      handlers->on_draw(&g_game);
    }

    plat_flip(g_game.screen);

    /* Inverted delay: it shortens as frames get slower, so it is a feedback
     * loop rather than a frame limiter. Kept as-is; Slice 5 replaces it with
     * a real fixed timestep. */
    if (g_game.dt > 0) {
      plat_delay(300 / g_game.dt);
    } else {
      plat_delay(16);
    }

    /* Transitions run between frames, so a handler can request one without
     * its remaining work being skipped. */
    if (g_game.next != g_game.state) {
      const game_state_handlers *next_handlers;
      if (g_game.next == ST_QUIT) {
        g_game.running = 0;
        continue;
      }
      if (handlers != NULL && handlers->on_exit != NULL) {
        handlers->on_exit(&g_game);
      }
      g_game.state = g_game.next;
      next_handlers = handlers_for(g_game.state);
      if (next_handlers != NULL && next_handlers->on_enter != NULL) {
        next_handlers->on_enter(&g_game);
      }
    }
  }
}

void cleanup_game(void) {
  game_assets *art = &g_game.art;
  int i, j;

  librer(art->background);
  librer(art->B_play);
  librer(art->B_play1);
  librer(art->B_settings);
  librer(art->B_settings1);
  librer(art->B_quit);
  librer(art->B_quit1);
  librer(art->settings);
  librer(art->slayed);
  librer(art->exits);
  librer(art->exits1);

  /* Level 1 scene art. */
  librer(art->backgame);
  librer(art->backgame_near);
  librer(art->obs_crate);
  librer(art->obs_crate_tall);
  librer(art->goal_beacon);
  librer(art->ground_tile);

  plat_sound_free(art->click);

  plat_font_close(g_game.p.police_score);
  plat_font_close(g_game.p1.police_score);

  /* image[2][9] is declared but only [2][7] is ever filled, so the bounds
   * here must stay in step with the load loops in src/perso.c. */
  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 7; ++j) {
      plat_image_free(g_game.p.image[i][j]);
      plat_image_free(g_game.p1.image[i][j]);
    }
  }

  if (g_game.p.barre != NULL) {
    for (i = 0; i < 6; ++i) {
      plat_image_free(g_game.p.barre[i]);
    }
    free(g_game.p.barre);
  }
  if (g_game.p1.barre != NULL) {
    for (i = 0; i < 6; ++i) {
      plat_image_free(g_game.p1.barre[i]);
    }
    free(g_game.p1.barre);
  }

  plat_image_free(g_game.p.score);
  plat_image_free(g_game.p1.score);

  plat_shutdown();
}

int main(void) {
  if (!init_engine()) {
    return EXIT_FAILURE;
  }
  load_game_resources();
  run_game_loop();
  cleanup_game();
  return EXIT_SUCCESS;
}
