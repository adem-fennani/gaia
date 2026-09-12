/* Level 1 heads-up display. See include/hud.h for the layout rationale. */

#include "../include/hud.h"
#include <stdio.h>

/* Map a world x onto the minimap's drawable span. */
static int minimap_x(int world_x) {
  const int pad = 6;
  int span = HUD_MAP_W - 2 * pad;
  int x = world_x;
  if (x < 0) {
    x = 0;
  }
  if (x > LEVEL1_W) {
    x = LEVEL1_W;
  }
  return HUD_MAP_X + pad + (x * span / LEVEL1_W);
}

/* Full health, and the number of barre frames. */
#define HUD_VIE_MAX 5

static void draw_bar(game_ctx *ctx, const perso *player, int x) {
  plat_rect dst;
  int health = player->vie;
  int frame;
  if (player->barre == NULL) {
    return;
  }
  if (health < 0) {
    health = 0;
  }
  if (health > HUD_VIE_MAX) {
    health = HUD_VIE_MAX;
  }
  /* The art is indexed by damage taken, not by health: barre_0 is five red
   * hearts (full) and barre_5 is five empty ones. The old code blitted
   * barre[vie] directly, which is inverted -- and because vie was never
   * decremented anywhere, only barre[5] ever rendered, so the HUD showed an
   * empty bar while both players were at full health. */
  frame = HUD_VIE_MAX - health;
  if (player->barre[frame] == NULL) {
    return;
  }
  dst.x = (Sint16)x;
  dst.y = HUD_TOP;
  dst.w = HUD_BAR_W;
  dst.h = HUD_BAR_H;
  plat_blit(player->barre[frame], NULL, ctx->screen, &dst);
}

static void draw_minimap(game_ctx *ctx) {
  plat_rect dst;
  int i;

  if (ctx->art.minimap.img != NULL) {
    dst.x = HUD_MAP_X;
    dst.y = HUD_TOP;
    dst.w = HUD_MAP_W;
    dst.h = HUD_MAP_H;
    plat_blit(ctx->art.minimap.img, NULL, ctx->screen, &dst);
  }

  /* One marker per player, positioned from world coordinates every frame.
   * The old MAJminimap() took an incremental step and a direction code, was
   * never called, and had no bounds clamp, so the arrow stayed frozen at
   * (576,94) while being blitted every frame. Deriving the position directly
   * is both correct and less code; MAJminimap goes away with the rest of the
   * dead minimap helpers.
   *
   * Each marker points down, so its tip is the position. They occupy two
   * fixed rows in the map's upper lane, clear of the terrain pips. */
  for (i = 0; i < 2; ++i) {
    const image *art = (i == 0) ? &ctx->art.marker_p1 : &ctx->art.marker_p2;
    const perso *player = (i == 0) ? &ctx->p : &ctx->p1;
    plat_rect marker;
    if (art->img == NULL) {
      continue;
    }
    marker.x = (Sint16)(minimap_x(player->pos_background.x) - HUD_MARKER / 2);
    marker.y = (Sint16)(HUD_TOP + 4 + i * (HUD_MARKER + 1));
    marker.w = HUD_MARKER;
    marker.h = HUD_MARKER;
    plat_blit(art->img, NULL, ctx->screen, &marker);
  }
}

static void draw_timer(game_ctx *ctx, plat_color color) {
  char line[32];
  Uint32 elapsed_ms = plat_ticks() - ctx->level1_start_ticks;
  int seconds = (int)(elapsed_ms / 1000);
  int text_w = 0;

  /* Elapsed since the level began, not absolute ticks. The old MAJ_temps()
   * computed SDL_GetTicks()/1000, so it showed time since SDL_Init; its
   * leading-zero if-chain also left min>=10 && sec>=10 uncovered, which left
   * stale text on screen, and it re-rendered into text.surf every call without
   * freeing the previous surface. snprintf and plat_text_draw avoid all three:
   * %02d needs no branches and plat_text_draw frees what it renders. */
  snprintf(line, sizeof(line), "%02d:%02d", seconds / 60, seconds % 60);
  if (ctx->art.hud_font != NULL) {
    TTF_SizeText(ctx->art.hud_font, line, &text_w, NULL);
  }
  plat_text_draw(ctx->screen, ctx->art.hud_font, line,
                 HUD_MAP_X + (HUD_MAP_W - text_w) / 2, HUD_ROW2, color);
}

void hud_draw(game_ctx *ctx) {
  plat_color white = {255, 255, 255, 0};
  plat_color yellow = {255, 255, 0, 0};
  char line[64];

  draw_bar(ctx, &ctx->p, HUD_PAD);
  draw_bar(ctx, &ctx->p1, HUD_P2_X);

  snprintf(line, sizeof(line), "P1 score %d", ctx->p.iscore);
  plat_text_draw(ctx->screen, ctx->art.hud_font, line, HUD_PAD, HUD_ROW2,
                 white);
  snprintf(line, sizeof(line), "P2 score %d", ctx->p1.iscore);
  plat_text_draw(ctx->screen, ctx->art.hud_font, line, HUD_P2_X, HUD_ROW2,
                 white);

  draw_minimap(ctx);
  draw_timer(ctx, yellow);

  plat_text_draw(ctx->screen, ctx->art.hud_font,
                 "Reach the exit beacon to finish the mission", HUD_PAD,
                 HUD_ROW3, yellow);
  plat_text_draw(ctx->screen, ctx->art.hud_font,
                 "P1: arrows, space to jump, A/D to run    "
                 "P2: K/M, J to jump, O up, L down",
                 HUD_PAD, HUD_ROW4, white);
}
