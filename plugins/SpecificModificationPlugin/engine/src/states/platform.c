#pragma bank 255

#include <rand.h>

#include "data/states_defines.h"
#include "states/platform.h"

#include "gbs_types.h"
#include "actor.h"
#include "camera.h"
#include "collision.h"
#include "data_manager.h"
#include "game_time.h"
#include "input.h"
#include "math.h"
#include "scroll.h"
#include "trigger.h"
#include "vm.h"

#ifndef INPUT_PLATFORM_JUMP
#define INPUT_PLATFORM_JUMP        INPUT_A
#endif
#ifndef INPUT_PLATFORM_RUN
#define INPUT_PLATFORM_RUN         INPUT_B
#endif
#ifndef INPUT_PLATFORM_INTERACT
#define INPUT_PLATFORM_INTERACT    INPUT_A
#endif
#ifndef PLATFORM_CAMERA_DEADZONE_X
#define PLATFORM_CAMERA_DEADZONE_X 4
#endif
#ifndef PLATFORM_CAMERA_DEADZONE_Y
#define PLATFORM_CAMERA_DEADZONE_Y 16
#endif
#ifndef COLLISION_SLOPE_45_RIGHT
#define COLLISION_SLOPE_45_RIGHT 0x20
#endif
#ifndef COLLISION_SLOPE_225_RIGHT_BOT
#define COLLISION_SLOPE_225_RIGHT_BOT 0x40
#endif
#ifndef COLLISION_SLOPE_225_RIGHT_TOP
#define COLLISION_SLOPE_225_RIGHT_TOP 0x60
#endif
#ifndef COLLISION_SLOPE_45_LEFT
#define COLLISION_SLOPE_45_LEFT 0x30
#endif
#ifndef COLLISION_SLOPE_225_LEFT_BOT
#define COLLISION_SLOPE_225_LEFT_BOT 0x50
#endif
#ifndef COLLISION_SLOPE_225_LEFT_TOP
#define COLLISION_SLOPE_225_LEFT_TOP 0x70
#endif
#ifndef COLLISION_SLOPE
#define COLLISION_SLOPE 0xF0
#endif

#define IS_ON_SLOPE(t) ((t) & 0x60)
#define IS_SLOPE_LEFT(t) ((t) & 0x10)
#define IS_SLOPE_RIGHT(t) (((t) & 0x10) == 0)

platform_actor_t p1_actor;
platform_actor_t p2_actor;
platform_actor_t *current_p_actor;

WORD plat_min_vel;
WORD plat_walk_vel;
WORD plat_climb_vel;
WORD plat_run_vel;
WORD plat_walk_acc;
WORD plat_run_acc;
WORD plat_dec;
WORD plat_jump_vel;
WORD plat_grav;
WORD plat_hold_grav;
WORD plat_max_fall_vel;

void platform_init(void) BANKED {    
    camera_offset_x = 0;
    camera_offset_y = 0;
    camera_deadzone_x = PLATFORM_CAMERA_DEADZONE_X;
    camera_deadzone_y = PLATFORM_CAMERA_DEADZONE_Y;
	camera_settings = 0;
    game_time = 0;
	p1_actor.enemy_actor = &p2_actor;
	p2_actor.enemy_actor = &p1_actor;
}

void update_current_actor(void) BANKED {
	actor_t *current_actor = current_p_actor->actor;
	if (!current_actor){
		return;
	}
	//update input
	if (current_p_actor->is_player){
		current_p_actor->frame_input = frame_joy;
		current_p_actor->last_input = last_joy;
		current_p_actor->recent_input = recent_joy;
	} else {
		//TODO: make CPU logic here
		//if current actor is IDLE
		//if current actor is not facing enemy actor
			//press direction toward enemy actor
		//else
			//release direction (random after x frames)
		//if enemy actor is within attack range
			//if enemy actor has been defending for x frames
				//press jump + forward or charge (random)
			//else
				//press attack
		//else 
			/*
		current_p_actor->last_input = current_p_actor->frame_input;
		
		if (current_p_actor->input_counter == 0){
			current_p_actor->frame_input = rand();
			current_p_actor->input_counter = 4 + (rand() & 15);
		}
		
		if ((current_p_actor->frame_input ^ current_p_actor->last_input) & INPUT_DPAD)
			current_p_actor->recent_input = ((current_p_actor->frame_input & ~current_p_actor->last_input) & INPUT_DPAD);
		
		if (!(game_time & 15)){
			current_p_actor->input_counter--;
		}
		*/
	}
	
	
    UBYTE tile_start, tile_end;
    actor_t *hit_actor;
		
    UBYTE p_half_width = (current_actor->bounds.right - current_actor->bounds.left) >> 1;
    UBYTE tile_x_mid = ((current_actor->pos.x >> 4) + current_actor->bounds.left + p_half_width) >> 3; 
    UBYTE tile_y = ((current_actor->pos.y >> 4) + current_actor->bounds.top + 1) >> 3;

    // Horizontal Movement
    if (P_INPUT_LEFT) {
        if (P_INPUT_B) {
            current_p_actor->vel_x -= plat_run_acc;
            current_p_actor->vel_x = CLAMP(current_p_actor->vel_x, -plat_run_vel, -plat_min_vel);
        } else {
            current_p_actor->vel_x -= plat_walk_acc;
            current_p_actor->vel_x = CLAMP(current_p_actor->vel_x, -plat_walk_vel, -plat_min_vel);
        } 
    } else if (P_INPUT_RIGHT) {
        if (P_INPUT_B) {
            current_p_actor->vel_x += plat_run_acc;
            current_p_actor->vel_x = CLAMP(current_p_actor->vel_x, plat_min_vel, plat_run_vel);
        } else {
            current_p_actor->vel_x += plat_walk_acc;
            current_p_actor->vel_x = CLAMP(current_p_actor->vel_x, plat_min_vel, plat_walk_vel);
        }
    } else if (current_p_actor->grounded) {
        if (current_p_actor->vel_x < 0) {
            current_p_actor->vel_x += plat_dec;
            if (current_p_actor->vel_x > 0) {
                current_p_actor->vel_x = 0;
            }
        } else if (current_p_actor->vel_x > 0) {
            current_p_actor->vel_x -= plat_dec;
            if (current_p_actor->vel_x < 0) {
                current_p_actor->vel_x = 0;
            }
        }
    }

    UBYTE col = 0;
    // Vertical Movement

    // Gravity
    if (P_INPUT_A && current_p_actor->vel_y < 0) {
        current_p_actor->vel_y += plat_hold_grav;
    } else {
        current_p_actor->vel_y += plat_grav;
    }

    // Step X
    UBYTE prev_on_slope = current_p_actor->on_slope;
    current_p_actor->on_slope = FALSE;
    tile_start = (((current_actor->pos.y >> 4) + current_actor->bounds.top)    >> 3);
    tile_end   = (((current_actor->pos.y >> 4) + current_actor->bounds.bottom) >> 3) + 1;
    UWORD old_x = current_actor->pos.x;
    WORD new_x = current_actor->pos.x + (current_p_actor->vel_x >> 8);
    UBYTE tile_x = 0;
    UBYTE col_mid = 0;
    if (current_p_actor->vel_x > 0) {
        tile_x = ((new_x >> 4) + current_actor->bounds.right) >> 3;
        tile_y   = (((current_actor->pos.y >> 4) + current_actor->bounds.bottom) >> 3);
        UBYTE tile_x_mid = ((new_x >> 4) + current_actor->bounds.left + p_half_width + 1) >> 3; 
        col_mid = tile_at(tile_x_mid, tile_y);
        if (IS_ON_SLOPE(col_mid)) {
            current_p_actor->on_slope = col_mid;
            current_p_actor->slope_y = tile_y;
        }

        UBYTE slope_on_y = FALSE;
        while (tile_start != tile_end) {
            col = tile_at(tile_x, tile_start);
            if (IS_ON_SLOPE(col)) {
                slope_on_y = TRUE;
            }

            if (col & COLLISION_LEFT) {
                // only ignore collisions if there is a slope on this y column somewhere
                if (slope_on_y || tile_start == current_p_actor->slope_y) {
                    // Right slope
                    if ((IS_ON_SLOPE(current_p_actor->on_slope) && IS_SLOPE_RIGHT(current_p_actor->on_slope)) ||
                        (IS_ON_SLOPE(prev_on_slope) && IS_SLOPE_RIGHT(prev_on_slope))
                        )
                        {
                        if (tile_start <= current_p_actor->slope_y) {
                            tile_start++;
                            continue;
                        }
                    }
                }
                if (slope_on_y) {
                    // Left slope
                    if ((IS_ON_SLOPE(current_p_actor->on_slope) && IS_SLOPE_LEFT(current_p_actor->on_slope)) ||
                        (IS_ON_SLOPE(prev_on_slope) && IS_SLOPE_LEFT(prev_on_slope))
                        )
                        {
                        if (tile_start >= current_p_actor->slope_y) {
                            tile_start++;
                            continue;
                        }
                    }
                }
                new_x = (((tile_x << 3) - current_actor->bounds.right) << 4) - 1;
                current_p_actor->vel_x = 0;
                break;
            }
            tile_start++;
        }
        current_actor->pos.x = MIN((image_width - current_actor->bounds.right - 1) << 4, new_x);
    } else if (current_p_actor->vel_x < 0) {
        tile_x = ((new_x >> 4) + current_actor->bounds.left) >> 3;
        tile_y   = (((current_actor->pos.y >> 4) + current_actor->bounds.bottom) >> 3);
        UBYTE tile_x_mid = ((new_x >> 4) + current_actor->bounds.left + p_half_width + 1) >> 3; 
        col_mid = tile_at(tile_x_mid, tile_y);
        if (IS_ON_SLOPE(col_mid)) {
            current_p_actor->on_slope = col_mid;
            current_p_actor->slope_y = tile_y;
        }

        tile_start = (((current_actor->pos.y >> 4) + current_actor->bounds.top)    >> 3);
        UBYTE slope_on_y = FALSE;
        while (tile_start != tile_end) {
            col = tile_at(tile_x, tile_start);
            if (IS_ON_SLOPE(col)) {
                slope_on_y = TRUE;
            }

            if (col & COLLISION_RIGHT) {
                // only ignore collisions if there is a slope on this y column somewhere
                if (slope_on_y || tile_start == current_p_actor->slope_y) {
                    // Left slope
                    if ((IS_ON_SLOPE(current_p_actor->on_slope) && IS_SLOPE_LEFT(current_p_actor->on_slope)) ||
                        (IS_ON_SLOPE(prev_on_slope) && IS_SLOPE_LEFT(prev_on_slope))                            
                        )
                        {
                        if (tile_start <= current_p_actor->slope_y) {
                            tile_start++;
                            continue;
                        }
                    }
                }
                if (slope_on_y) {
                    // Right slope
                    if ((IS_ON_SLOPE(current_p_actor->on_slope) && IS_SLOPE_RIGHT(current_p_actor->on_slope)) ||
                        (IS_ON_SLOPE(prev_on_slope) && IS_SLOPE_RIGHT(prev_on_slope))
                        )
                        {
                        if (tile_start >= current_p_actor->slope_y) {
                            tile_start++;
                            continue;
                        }
                    }
                }
                new_x = ((((tile_x + 1) << 3) - current_actor->bounds.left) << 4) + 1;
                current_p_actor->vel_x = 0;
                break;
            }
            tile_start++;
        }
        current_actor->pos.x = MAX(0, new_x);
    }

    // Step Y
    UBYTE prev_grounded = current_p_actor->grounded;
    UWORD old_y = current_actor->pos.y;
    current_p_actor->grounded = FALSE;
    // 1 frame leniency of current_p_actor->grounded state if we were on a slope last frame
    if (prev_on_slope) current_p_actor->grounded = TRUE;
    tile_start = (((current_actor->pos.x >> 4) + current_actor->bounds.left)  >> 3);
    tile_end   = (((current_actor->pos.x >> 4) + current_actor->bounds.right) >> 3) + 1;
    if (current_p_actor->vel_y > 0) {
        UWORD new_y = current_actor->pos.y + (current_p_actor->vel_y >> 8);
        tile_y = (((current_actor->pos.y >> 4) + current_actor->bounds.bottom) >> 3) - 1;
        UBYTE new_tile_y = ((new_y >> 4) + current_actor->bounds.bottom) >> 3;
        // If previously current_p_actor->grounded and gravity is not enough to pull us down to the next tile, manually check it for the next slope
        // This prevents the "animation glitch" when going down slopes
        if (prev_grounded && new_tile_y == (tile_y + 1)) new_tile_y += 1;
        UWORD x_mid_coord = ((current_actor->pos.x >> 4) + current_actor->bounds.left + p_half_width + 1);
        while (tile_y <= new_tile_y) {
            UBYTE col = tile_at(x_mid_coord >> 3, tile_y);
            UWORD tile_x_coord = (x_mid_coord >> 3) << 3;
            UWORD x_offset = x_mid_coord - tile_x_coord;
            UWORD slope_y_coord = 0;
            if (IS_ON_SLOPE(col)) {
                if ((col & COLLISION_SLOPE) == COLLISION_SLOPE_45_RIGHT) {
                    slope_y_coord = (((tile_y << 3) + (8 - x_offset) - current_actor->bounds.bottom) << 4) - 1;
                } else if ((col & COLLISION_SLOPE) == COLLISION_SLOPE_225_RIGHT_BOT) {
                    slope_y_coord = (((tile_y << 3) + (8 - (x_offset >> 1)) - current_actor->bounds.bottom) << 4) - 1;
                } else if ((col & COLLISION_SLOPE) == COLLISION_SLOPE_225_RIGHT_TOP) {
                    slope_y_coord = (((tile_y << 3) + (4 - (x_offset >> 1)) - current_actor->bounds.bottom) << 4) - 1;
                }

                else if ((col & COLLISION_SLOPE) == COLLISION_SLOPE_45_LEFT) {
                    slope_y_coord = (((tile_y << 3) + (x_offset) - current_actor->bounds.bottom) << 4) - 1;
                } else if ((col & COLLISION_SLOPE) == COLLISION_SLOPE_225_LEFT_BOT) {
                    slope_y_coord = (((tile_y << 3) + (x_offset >> 1) - current_actor->bounds.bottom + 4) << 4) - 1;
                } else if ((col & COLLISION_SLOPE) == COLLISION_SLOPE_225_LEFT_TOP) {
                    slope_y_coord = (((tile_y << 3) + (x_offset >> 1) - current_actor->bounds.bottom) << 4) - 1;
                }
            }

            if (slope_y_coord) {
                // If going downwards into a slope, don't snap to it unless we've actually collided
                if (!prev_grounded && slope_y_coord > new_y) {
                    tile_y++;
                    continue;
                }
                // If we are moving up a slope, check for top collision
                UBYTE slope_top_tile_y = (((slope_y_coord >> 4) + current_actor->bounds.top) >> 3);
                while (tile_start != tile_end) {
                    if (tile_at(tile_start, slope_top_tile_y) & COLLISION_BOTTOM) {
                        current_p_actor->vel_y = 0;
                        current_p_actor->vel_x = 0;
                        current_actor->pos.y = old_y;
                        current_actor->pos.x = old_x;
                        current_p_actor->grounded = TRUE;
                        current_p_actor->on_slope = col;
                        current_p_actor->slope_y = tile_y;
                        goto end_y_collision;
                    }
                    tile_start++;
                }

                current_actor->pos.y = slope_y_coord;
                current_p_actor->vel_y = 0;
                current_p_actor->grounded = TRUE;
                current_p_actor->on_slope = col;
                current_p_actor->slope_y = tile_y;
                goto end_y_collision;
            }
            tile_y++;
        }

        tile_start = (((current_actor->pos.x >> 4) + current_actor->bounds.left)  >> 3);
        tile_end   = (((current_actor->pos.x >> 4) + current_actor->bounds.right) >> 3) + 1;
        tile_y = ((new_y >> 4) + current_actor->bounds.bottom) >> 3;
        while (tile_start != tile_end) {
            // only snap to the top of a platform if feet are above the line
            if (tile_at(tile_start, tile_y) & COLLISION_TOP && ((current_actor->pos.y >> 4) + current_actor->bounds.bottom - 2) < (tile_y << 3) ) {
                new_y = ((((tile_y) << 3) - current_actor->bounds.bottom) << 4) - 1;
                current_p_actor->grounded = TRUE;
                current_p_actor->vel_y = 0;
                break;
            }
            tile_start++;
        }
        current_actor->pos.y = new_y;
    } else if (current_p_actor->vel_y < 0) {
        UWORD new_y = current_actor->pos.y + (current_p_actor->vel_y >> 8);
        tile_y = (((new_y >> 4) + current_actor->bounds.top) >> 3);
        while (tile_start != tile_end) {
            if (tile_at(tile_start, tile_y) & COLLISION_BOTTOM) {
                new_y = ((((UBYTE)(tile_y + 1) << 3) - current_actor->bounds.top) << 4) + 1;
                current_p_actor->vel_y = 0;
                break;
            }
            tile_start++;
        }
        current_actor->pos.y = new_y;
    }
end_y_collision:

    // Clamp Y Velocity
    current_p_actor->vel_y = MIN(current_p_actor->vel_y, plat_max_fall_vel);
    

    // Check for trigger collisions
    if (trigger_activate_at_intersection(&current_actor->bounds, &current_actor->pos, FALSE)) {
        // Landed on a trigger
        return;
    }

    // Actor Collisions
    UBYTE can_jump = TRUE;
	if (current_actor->script.bank){
		hit_actor = actor_overlapping_bb(&current_actor->bounds, &current_actor->pos, current_actor, FALSE);
		if (hit_actor != NULL && hit_actor->collision_group) {
			script_execute(current_actor->script.bank, current_actor->script.ptr, 0, 1, (UWORD)(hit_actor->collision_group));
		}
	}
    // Jump
    if (P_INPUT_A_PRESSED && current_p_actor->grounded && can_jump) {
        current_p_actor->vel_y = -plat_jump_vel;
        current_p_actor->grounded = FALSE;
    }

    // current_actor animation
    if (current_p_actor->grounded) {
		if (current_p_actor->vel_x != 0){
			if (current_p_actor->enemy_actor->actor->pos.x < current_p_actor->actor->pos.x) {
				actor_set_dir(current_actor, DIR_LEFT, TRUE);
			} else {
				actor_set_dir(current_actor, DIR_RIGHT, TRUE);
			}
        } else {
            actor_set_anim_idle(current_actor);
        }
    } else {
        if (current_p_actor->enemy_actor->actor->pos.x < current_p_actor->actor->pos.x) {
            actor_set_anim(current_actor, ANIM_JUMP_LEFT);
        } else {
            actor_set_anim(current_actor, ANIM_JUMP_RIGHT);
        }
    }
	
}

void platform_update(void) BANKED {
	
	current_p_actor = &p1_actor;
	update_current_actor();
	current_p_actor = &p2_actor;
	update_current_actor();
	//camera update
	if (p1_actor.actor && p2_actor.actor) {	
		UWORD a_x = ((p1_actor.actor->pos.x + p2_actor.actor->pos.x) >> 1) + 128;
		// Horizontal lock
		if (camera_x < a_x - (camera_deadzone_x << 4) - (camera_offset_x << 4)) { 
			camera_x = a_x - (camera_deadzone_x << 4) - (camera_offset_x << 4);
		} else if (camera_x > a_x + (camera_deadzone_x << 4) - (camera_offset_x << 4)) { 
			camera_x = a_x + (camera_deadzone_x << 4) - (camera_offset_x << 4);
		}
	
		UWORD a_y = ((p1_actor.actor->pos.y + p2_actor.actor->pos.y) >> 1) + 128;
		// Vertical lock
		if (camera_y < a_y - (camera_deadzone_y << 4) - (camera_offset_y << 4)) { 
			camera_y = a_y - (camera_deadzone_y << 4) - (camera_offset_y << 4);
		} else if (camera_y > a_y + (camera_deadzone_y << 4) - (camera_offset_y << 4)) { 
			camera_y = a_y + (camera_deadzone_y << 4) - (camera_offset_y << 4);
		}
	}
}

void vm_set_platformer_actor(SCRIPT_CTX * THIS) OLDCALL BANKED {
    UBYTE actor_idx = *(uint8_t *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE player_idx = *(uint8_t *)VM_REF_TO_PTR(FN_ARG1);
	current_p_actor = (player_idx)? &p2_actor: &p1_actor;
	current_p_actor->actor = &actors[actor_idx];
	current_p_actor->is_player = (current_p_actor->actor == &PLAYER)? 1: 0;
}

void vm_set_simulated_input(SCRIPT_CTX * THIS) OLDCALL BANKED {
	UBYTE new_input = *(int8_t*)VM_REF_TO_PTR(FN_ARG0);
	UBYTE player_idx = *(uint8_t *)VM_REF_TO_PTR(FN_ARG1);	
	current_p_actor = (player_idx)? &p2_actor: &p1_actor;
	current_p_actor->last_input = current_p_actor->frame_input;
	current_p_actor->frame_input = new_input;
	if ((current_p_actor->frame_input ^ current_p_actor->last_input) & INPUT_DPAD)
		current_p_actor->recent_input = ((current_p_actor->frame_input & ~current_p_actor->last_input) & INPUT_DPAD);
}