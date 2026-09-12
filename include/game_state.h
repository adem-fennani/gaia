#ifndef GAME_STATE_H_INCLUDED
#define GAME_STATE_H_INCLUDED

/* The game's state machine.
 *
 * Replaces the `while (done) switch (etat)` loop and the ~20 file-scope
 * statics that used to live in main_menu.c. Each state is a module exporting a
 * handler table; src/game.c owns the context and drives transitions.
 *
 * Every state gets the same lifecycle:
 *
 *   on_enter   once, when the state becomes current
 *   on_event   per SDL event, while current
 *   on_update  once per frame, before drawing
 *   on_draw    once per frame
 *   on_exit    once, when leaving
 *
 * Any handler may be NULL. Transitions are requested with game_request(),
 * never by assigning the current state, so on_exit/on_enter always run in
 * pairs. That is what fixes the old asymmetry where entering level 1 from the
 * menu reset it but returning to it after ESC did not.
 */

#include "menu.h"
#include "minimap.h"
#include "perso.h"
#include "platform.h"

#define SCREEN_W 1150
#define SCREEN_H 650

/* LEVEL1_W and LEVEL1_GROUND_STRIP_Y come from perso.h. */
#define LEVEL1_OBSTACLE_COUNT 3

/* Physics timestep, in seconds, and the largest frame the accumulator will
 * absorb. Capping is what stops a stall from queueing a catch-up burst. */
#define LEVEL1_STEP (1.0 / 60.0)
#define LEVEL1_MAX_FRAME 0.10

/* Frame pacing. TARGET_FRAME_MS is the budget the loop sleeps out to;
 * FRAME_DT_CAP_MS bounds what a stalled frame can report as elapsed. */
#define TARGET_FRAME_MS 16
#define FRAME_DT_CAP_MS 100
#define GROUND_TILE_W 64

#define VICTORY_AUTO_RETURN_MS 4000

/* Parallax rates. The backdrop layers are generated at exactly the width
 * these rates require; scripts/verify_assets.py enforces that relationship. */
#define PARALLAX_FAR_NUM 1
#define PARALLAX_FAR_DEN 2
#define PARALLAX_NEAR_NUM 3
#define PARALLAX_NEAR_DEN 4

typedef enum {
  ST_MENU = 0,
  ST_LEVEL1,
  ST_SETTINGS,
  ST_VICTORY,
  ST_QUIT
} game_state;

/* Everything loaded once at startup and freed once at shutdown. All states
 * share it, because load_game_resources() still loads every asset for every
 * state up front; per-state loading is 0.5.0 work. */
typedef struct {
  image background;
  image B_play, B_play1;
  image B_settings, B_settings1;
  image B_quit, B_quit1;
  image settings;
  image slayed;
  image exits, exits1;

  image backgame;       /* level 1 far parallax layer */
  image backgame_near;  /* level 1 near parallax layer */
  image obs_crate;      /* obstacles [0] and [2] */
  image obs_crate_tall; /* obstacle [1] */
  image goal_beacon;
  image ground_tile;

  image marker_p1; /* minimap position markers, one per player */
  image marker_p2;

  plat_sound *click;

  /* HUD text. The 40pt perso.police_score is kept for the victory screen,
   * where there is room for it; at 40pt the HUD lines overlapped each other. */
  plat_font *hud_font;
} game_assets;

typedef struct {
  plat_surface *screen;
  game_assets art;

  game_state state;
  game_state next; /* pending transition; == state means none */
  int running;

  perso p, p1;
  minimap map;
  plat_rect camera;

  /* Level 1 geometry, in world space. */
  plat_rect obstacles[LEVEL1_OBSTACLE_COUNT];
  plat_rect goal;

  /* Per-player input flags. dep = a move key is held, acc = 1 to run forward
   * / 2 to run backward. The old posy/posy1 scalars are gone: the floor is
   * now found by collision, not captured on the jump keypress. */
  int dep, acc;
  int dep1, acc1;

  /* Real elapsed ms for the last frame, and the tick it was measured from. */
  Uint32 dt, t_prev;
  /* Unconsumed time carried into the next physics step. */
  double step_acc;

  /* Menu and settings. */
  int hover;      /* highlighted button, 1-3, or 0 for none */
  int exit_hover; /* settings back button hovered */
  int volume;
  plat_rect pos_plus, pos_moin;

  /* Victory stats, frozen when the mission completes. */
  Uint32 level1_start_ticks;
  Uint32 victory_ticks;
  int victory_score_p, victory_score_p1;
  int victory_time_sec;
  int victory_winner; /* 0 none, 1 p, 2 p1, 3 both */
} game_ctx;

typedef struct {
  void (*on_enter)(game_ctx *ctx);
  void (*on_event)(game_ctx *ctx, const SDL_Event *event);
  void (*on_update)(game_ctx *ctx);
  void (*on_draw)(game_ctx *ctx);
  void (*on_exit)(game_ctx *ctx);
} game_state_handlers;

extern const game_state_handlers menu_state;
extern const game_state_handlers level1_state;
extern const game_state_handlers settings_state;
extern const game_state_handlers victory_state;

/* Request a transition. Takes effect after the current frame finishes, so a
 * handler can ask to leave without its remaining work being skipped. */
void game_request(game_ctx *ctx, game_state next);

/* True when the point is inside the image's drawn area. The menu and settings
 * screens both hit-test buttons this way. */
int image_contains(const image *img, int x, int y);

#endif
