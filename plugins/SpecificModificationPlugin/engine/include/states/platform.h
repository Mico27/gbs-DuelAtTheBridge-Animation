#ifndef STATE_PLATFORM_H
#define STATE_PLATFORM_H

#include <gbdk/platform.h>
#include "gbs_types.h"

typedef enum {
    IDLE = 0,
	MOVING = 1,
    JUMP = 2,
    FALL = 3,
} platform_actor_sub_state_e;

typedef enum {
    DEFAULT = 0,
	ATTACK = 1,
    CHARGE = 2,
    DEFEND = 3,
} platform_actor_state_e;

typedef struct platform_actor_t
{
	actor_t *actor;
	bounding_box_t hit_box;
	bounding_box_t hurt_box;
	
    UBYTE grounded;
	UBYTE on_slope;
	UBYTE slope_y;
	
	WORD vel_x;
	WORD vel_y;
	
	platform_actor_sub_state_e current_sub_state;
	platform_actor_sub_state_e next_sub_state;
	platform_actor_state_e current_state;
	platform_actor_state_e next_state;
	UBYTE hp;
	UBYTE iframe;
	UBYTE counter_a;
	UBYTE input_counter;
	
	UBYTE frame_input;
	UBYTE last_input;
	UBYTE recent_input;
	UBYTE is_player;
	
	struct platform_actor_t *enemy_actor;
	
} platform_actor_t;

/* TRUE if any button is being held */
#define P_INPUT_ANY (current_p_actor->frame_input)

/* TRUE if left is being held on dpad */
#define P_INPUT_LEFT (current_p_actor->frame_input & J_LEFT)

/* TRUE if right is being held on dpad */
#define P_INPUT_RIGHT (current_p_actor->frame_input & J_RIGHT)

/* TRUE if up is being held on dpad */
#define P_INPUT_UP (current_p_actor->frame_input & J_UP)

/* TRUE if down is being held on dpad */
#define P_INPUT_DOWN (current_p_actor->frame_input & J_DOWN)

/* TRUE if left is most recent direction being held on dpad */
#define P_INPUT_RECENT_LEFT ((current_p_actor->recent_input & J_LEFT) || (!current_p_actor->recent_input && (current_p_actor->frame_input & J_LEFT)))

/* TRUE if right is most recent direction being held on dpad */
#define P_INPUT_RECENT_RIGHT ((current_p_actor->recent_input & J_RIGHT) || (!current_p_actor->recent_input && (current_p_actor->frame_input & J_RIGHT)))

/* TRUE if up is most recent direction being held on dpad */
#define P_INPUT_RECENT_UP ((current_p_actor->recent_input & J_UP) || (!current_p_actor->recent_input && (current_p_actor->frame_input & J_UP)))

/* TRUE if down is most recent direction being held on dpad */
#define P_INPUT_RECENT_DOWN ((current_p_actor->recent_input & J_DOWN) || (!current_p_actor->recent_input && (current_p_actor->frame_input & J_DOWN)))

/* TRUE if A button is being held */
#define P_INPUT_A (current_p_actor->frame_input & J_A)

/* TRUE if B button is being held */
#define P_INPUT_B (current_p_actor->frame_input & J_B)

/* TRUE if A OR B button is being held */
#define P_INPUT_A_OR_B (current_p_actor->frame_input & (J_A | J_B))

/* TRUE if Start button is being held */
#define P_INPUT_START (current_p_actor->frame_input & J_START)

/* TRUE if Select button is being held */
#define P_INPUT_SELECT (current_p_actor->frame_input & J_SELECT)

/* TRUE on first frame that any button is pressed */
#define P_INPUT_ANY_PRESSED (current_p_actor->frame_input & ~current_p_actor->last_input)

/* TRUE on first frame that left is pressed on dpad  */
#define P_INPUT_LEFT_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & J_LEFT)

/* TRUE on first frame that right is pressed on dpad  */
#define P_INPUT_RIGHT_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & J_RIGHT)

/* TRUE on first frame that up is pressed on dpad  */
#define P_INPUT_UP_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & J_UP)

/* TRUE on first frame that down is pressed on dpad  */
#define P_INPUT_DOWN_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & J_DOWN)

/* TRUE on first frame that button is pressed */
#define P_INPUT_PRESSED(btn) ((current_p_actor->frame_input & ~current_p_actor->last_input) & (btn))

/* TRUE on first frame that A button is pressed */
#define P_INPUT_A_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & J_A)

/* TRUE on first frame that B button is pressed */
#define P_INPUT_B_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & J_B)

/* TRUE on first frame that A OR B button is pressed */
#define P_INPUT_A_OR_B_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & (J_A | J_B))

/* TRUE on first frame that Start button is pressed */
#define P_INPUT_START_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & J_START)

/* TRUE on first frame that Select button is pressed */
#define P_INPUT_SELECT_PRESSED ((current_p_actor->frame_input & ~current_p_actor->last_input) & J_SELECT)

#define P_INPUT_SOFT_RESTART (current_p_actor->frame_input == (J_A | J_B | J_START | J_SELECT))

/* resets the input */
#define P_INPUT_RESET (current_p_actor->last_input = current_p_actor->frame_input)

void platform_init(void) BANKED;
void platform_update(void) BANKED;

extern platform_actor_t p1_actor;
extern platform_actor_t p2_actor;
extern platform_actor_t *current_p_actor;
extern WORD plat_min_vel;
extern WORD plat_walk_vel;
extern WORD plat_run_vel;
extern WORD plat_climb_vel;
extern WORD plat_walk_acc;
extern WORD plat_run_acc;
extern WORD plat_dec;
extern WORD plat_jump_vel;
extern WORD plat_grav;
extern WORD plat_hold_grav;
extern WORD plat_max_fall_vel;

#endif
