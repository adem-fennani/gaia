#ifndef ENEMY_H_INCLUDED
#define ENEMY_H_INCLUDED

/* Patrolling enemies with contact damage.
 *
 * This is what finally gives the health bars a purpose: perso.vie was
 * initialised to 5 and never decremented anywhere in the tree, so barre[5] was
 * the only frame that had ever rendered and barre[0..4] were loaded dead.
 */

#include "collision.h"
#include "perso.h"

#define ENEMY_COUNT 3
#define ENEMY_FRAMES 6

/* Collision box, deliberately tighter than the 96x96 art so contact reads as
 * fair rather than as clipping a transparent corner. */
#define ENEMY_BOX_W 60
#define ENEMY_BOX_H 84

#define ENEMY_SPEED 90.0
#define ENEMY_ANIM_FPS 8.0

/* Grace period after taking a hit, so walking into an enemy costs one segment
 * rather than the whole bar in a handful of frames. */
#define PLAYER_INVULN_SECONDS 1.5
#define PLAYER_KNOCKBACK_X 280.0
#define PLAYER_KNOCKBACK_Y 420.0
/* Upward impulse given to a player who lands on an enemy. */
#define PLAYER_STOMP_BOUNCE 560.0

typedef struct {
  aabb body;
  int dir; /* 0 right, 1 left */
  double patrol_min, patrol_max;
  int frame;
  double anim_acc;
  int alive;
} enemy;

/* Places the three enemies on their patrol segments, alive and grounded. */
void enemy_reset_all(enemy *list);

/* Advances one enemy by dt seconds, turning at its patrol bounds. */
void enemy_step(enemy *e, double dt, const plat_rect *solids, int solid_count,
                double ground_y, double level_w);

/* Resolves contact between one player and the enemies.
 *
 * A player falling onto an enemy kills it and bounces; contact from any other
 * direction costs a health segment and knocks the player back. Returns 1 if
 * the player took damage. */
int enemy_collide_player(enemy *list, perso *player);

#endif
