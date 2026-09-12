#ifndef HUD_H_INCLUDED
#define HUD_H_INCLUDED

/* Level 1 heads-up display.
 *
 * The layout lives here as one set of constants, because previously every
 * element positioned itself independently and they all collided on the
 * 1150x650 screen:
 *
 *   - health bars were 455x104 at (0,0) and (575,0), and the three lines of
 *     HUD text were drawn at y=10/34/58, i.e. inside them
 *   - those three lines were themselves 40pt, ~40px tall, at 24px apart, so
 *     they overlapped each other too
 *   - the scores sat at y=70, also inside the bars
 *   - the minimap was 780x130 at (576,20): 206px wider than the screen, and
 *     straight over player 2's bar
 *
 * The HUD now owns the health bars and scores. afficherPerso() draws only the
 * character, which is also what stops it leaking a text surface per frame.
 */

#include "game_state.h"

/* All HUD art is sized to these; scripts/verify_assets.py enforces it. */
#define HUD_BAR_W 228
#define HUD_BAR_H 52
#define HUD_MAP_W 260
#define HUD_MAP_H 44
#define HUD_MARKER 11

/* One band across the top, clear of everything that scrolls. */
#define HUD_PAD 12
#define HUD_TOP 8
#define HUD_ROW2 (HUD_TOP + HUD_BAR_H + 4)
#define HUD_ROW3 (HUD_ROW2 + 22)
#define HUD_ROW4 (HUD_ROW3 + 20)
#define HUD_P2_X (SCREEN_W - HUD_BAR_W - HUD_PAD)
#define HUD_MAP_X ((SCREEN_W - HUD_MAP_W) / 2)

/* Point size for HUD text. The 40pt score font is kept for the victory
 * screen, where there is room for it. */
#define HUD_FONT_PT 16

void hud_draw(game_ctx *ctx);

#endif
