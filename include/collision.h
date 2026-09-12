#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED

/* Axis-separated swept AABB resolution.
 *
 * Replaces what level 1 had before: horizontal-only resolution plus a scalar
 * "ground height" captured by ypos_jump() at the moment the jump key was
 * pressed. That meant the floor was wherever the player's y happened to be
 * when they last jumped from standing, obstacles could not be landed on, and
 * there was no vertical response at all.
 *
 * Positions are doubles because the integer SDL_Rect the renderer wants
 * truncates every frame, and accumulating that truncation is what made the old
 * jump arc drift away from the parabola its own velocities implied.
 */

#include "platform.h"

typedef struct {
  double x, y; /* top-left of the box, world space */
  double w, h;
  double vy;     /* vertical velocity, px/s */
  int on_ground; /* set by the resolver; gates jumping */
} aabb;

/* Moves `body` by (dx, dy), resolving X then Y against `solids` and against
 * the ground plane at `ground_y`. X is clamped to [0, level_w - body->w].
 *
 * Resolving one axis at a time, each against the pre-move position on that
 * axis, is what allows landing on top of a solid: the Y pass sees a box that
 * is already horizontally clear, so "am I above it or beside it" is not
 * ambiguous.
 */
void collision_move(aabb *body, double dx, double dy, const plat_rect *solids,
                    int solid_count, double ground_y, double level_w);

#endif
