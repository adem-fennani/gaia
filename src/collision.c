#include "../include/collision.h"

static int overlaps(const aabb *body, const plat_rect *r) {
  return body->x < r->x + r->w && body->x + body->w > r->x &&
         body->y < r->y + r->h && body->y + body->h > r->y;
}

static void resolve_x(aabb *body, double dx, const plat_rect *solids,
                      int solid_count, double level_w) {
  int i;
  body->x += dx;

  if (body->x < 0) {
    body->x = 0;
  }
  if (body->x > level_w - body->w) {
    body->x = level_w - body->w;
  }

  if (dx == 0) {
    return;
  }
  for (i = 0; i < solid_count; ++i) {
    if (!overlaps(body, &solids[i])) {
      continue;
    }
    /* Push back to whichever face the motion came from. */
    if (dx > 0) {
      body->x = solids[i].x - body->w;
    } else {
      body->x = solids[i].x + solids[i].w;
    }
  }
}

static void resolve_y(aabb *body, double dy, const plat_rect *solids,
                      int solid_count, double ground_y) {
  int i;
  body->y += dy;
  body->on_ground = 0;

  for (i = 0; i < solid_count; ++i) {
    if (!overlaps(body, &solids[i])) {
      continue;
    }
    if (dy > 0) {
      /* Falling onto a solid: land on its top face. */
      body->y = solids[i].y - body->h;
      body->vy = 0;
      body->on_ground = 1;
    } else if (dy < 0) {
      /* Rising into a solid: stop against its underside. */
      body->y = solids[i].y + solids[i].h;
      body->vy = 0;
    }
  }

  /* The ground plane is the floor of the level; nothing falls through it. */
  if (body->y + body->h >= ground_y) {
    body->y = ground_y - body->h;
    body->vy = 0;
    body->on_ground = 1;
  }
}

void collision_move(aabb *body, double dx, double dy, const plat_rect *solids,
                    int solid_count, double ground_y, double level_w) {
  resolve_x(body, dx, solids, solid_count, level_w);
  resolve_y(body, dy, solids, solid_count, ground_y);
}
