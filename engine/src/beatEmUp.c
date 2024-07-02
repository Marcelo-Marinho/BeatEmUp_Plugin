#pragma bank 255

#include <gbdk/platform.h>

#include "data/states_defines.h"
#include "states/topdown.h"

//actor_set_dir
#include "actor.h"
#include "camera.h"
#include "collision.h"
//#include "data_manager.h"
#include "game_time.h"
#include "input.h"
#include "trigger.h"
#include "math.h"
#include "vm.h"
#include "vm_actor.h"

#ifndef INPUT_BEATEMUP_INTERACT
#define INPUT_BEATEMUP_INTERACT INPUT_A
#endif

UBYTE dir_anim = 1;
direction_e old_dir = 0;
UBYTE frame = 0;

void beatemup_init(void) BANKED {
    camera_offset_x = 0;
    camera_offset_y = 0;
    camera_deadzone_x = 0;
    camera_deadzone_y = 0;
    //PLAYER.frame = 0;
}

void beatemup_update(void) BANKED {
    actor_t *hit_actor;
    UBYTE tile_start, tile_end;
    direction_e new_dir = DIR_NONE;
    direction_e up_down_dir = DIR_NONE;
    bool up_down = false;
    

    // so stop movement for now
    player_moving = FALSE;

    // Check for trigger collisions
    if (trigger_activate_at_intersection(&PLAYER.bounds, &PLAYER.pos, FALSE)) {
        // Landed on a trigger
           return;
    }

    // Check input to set player movement
    if (INPUT_RECENT_LEFT) {
        player_moving = TRUE;
        new_dir = DIR_LEFT;
        dir_anim = 3;
        //up_down = false;

        if (INPUT_DOWN) {
            up_down = true;
            up_down_dir = DIR_DOWN;
        } else if (INPUT_UP) {
            up_down = true;
            up_down_dir = DIR_UP;
        }

        // Check for collisions to left of player
        tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top)    >> 3);
        tile_end   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 0;
        UBYTE tile_x = (((PLAYER.pos.x >> 4) + PLAYER.bounds.left) >> 3);
        while (tile_start != tile_end) {
            if (tile_at(tile_x - 0, tile_start) & COLLISION_RIGHT) {
                player_moving = FALSE;
                up_down = false;
                break;
            }
            tile_start++;
        }
    } else if (INPUT_RECENT_RIGHT) {
        player_moving = TRUE;
        new_dir = DIR_RIGHT;
        dir_anim = 1;
        //up_down = false;

        if (INPUT_DOWN) {
            up_down = true;
            up_down_dir = DIR_DOWN;
        } else if (INPUT_UP) {
            up_down = true;
            up_down_dir = DIR_UP;
        }

        // Check for collisions to right of player
        tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top)    >> 3);
        tile_end   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 0;
        UBYTE tile_x = ((PLAYER.pos.x >> 4) + PLAYER.bounds.right) >> 3;
        while (tile_start != tile_end) {
            if (tile_at(tile_x + 0, tile_start) & COLLISION_LEFT) {
                player_moving = FALSE;
                up_down = false;
                break;
            }
            tile_start++;
        }
    } 
    
    if (INPUT_RECENT_UP) {
        //player_moving = TRUE;
        up_down_dir = DIR_UP;
        new_dir = DIR_UP;
        up_down = true;

        if (INPUT_LEFT) {
            player_moving = TRUE;
            new_dir = DIR_LEFT;
        } else if (INPUT_RIGHT) {
            player_moving = TRUE;
            new_dir = DIR_RIGHT;
        }

        // Check for collisions below player
        tile_start = (((PLAYER.pos.x >> 4) + PLAYER.bounds.left)  >> 3);
        tile_end   = (((PLAYER.pos.x >> 4) + PLAYER.bounds.right) >> 3) + 0;
        UBYTE tile_y = ((PLAYER.pos.y >> 4) + PLAYER.bounds.top) >> 3;
        while (tile_start != tile_end) {
            if (tile_at(tile_start, tile_y - 0) & COLLISION_BOTTOM) {
                player_moving = FALSE;
                up_down = false;
                break;
            }
            tile_start++;
        }
    } else if (INPUT_RECENT_DOWN) {
        //player_moving = TRUE;
        up_down_dir = DIR_DOWN;
        new_dir = DIR_DOWN;
        up_down = true;

        if (INPUT_LEFT) {
            player_moving = TRUE;
            new_dir = DIR_LEFT;
        } else if (INPUT_RIGHT) {
            player_moving = TRUE;
            new_dir = DIR_RIGHT;
        }

        // Check for collisions below player
        tile_start = (((PLAYER.pos.x >> 4) + PLAYER.bounds.left)  >> 3);
        tile_end   = (((PLAYER.pos.x >> 4) + PLAYER.bounds.right) >> 3) + 0;
        UBYTE tile_y = ((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3;
        while (tile_start != tile_end) {
            if (tile_at(tile_start, tile_y + 0) & COLLISION_TOP) {
                player_moving = FALSE;
                up_down = false;
                break;
            }
            tile_start++;
        }
    }

    // Update direction
    if (new_dir != DIR_NONE) {
        actor_set_dir(&PLAYER, new_dir, false);
        actor_set_anim(&PLAYER, dir_anim + 4);
        actor_set_frame_offset(&PLAYER, frame);
        if((game_time & PLAYER.anim_tick) == 0) frame++;
        /*if(old_dir == 0) {
            actor_set_anim_moving(&PLAYER);
        }*/
        old_dir = 1;
    } else {
        actor_set_dir(&PLAYER, dir_anim, false);
        //actor_set_anim(&PLAYER, dir_anim);
        actor_set_frame_offset(&PLAYER, frame);
        if((game_time & PLAYER.anim_tick) == 0) frame++;
        actor_set_anim_idle(&PLAYER);
        old_dir = 0;
    }
   

    if (IS_FRAME_ODD) {
        // Check for actor overlap
        hit_actor = actor_overlapping_player(FALSE);
        if (hit_actor != NULL && hit_actor->collision_group) {
            player_register_collision_with(hit_actor);
        }
    }

    // Check if walked in to actor
    if (player_moving || up_down) {
        hit_actor = actor_in_front_of_player(1, FALSE);
        if (hit_actor != NULL) {
            player_register_collision_with(hit_actor);
            actor_stop_anim(&PLAYER);
            player_moving = FALSE;
            up_down = false;
        }
    }

    if (INPUT_PRESSED(INPUT_TOPDOWN_INTERACT)) {
        hit_actor = actor_in_front_of_player(8, TRUE);
        if (hit_actor != NULL && !hit_actor->collision_group) {
            actor_set_dir(hit_actor, FLIPPED_DIR(PLAYER.dir), FALSE);
            player_moving = FALSE;
            up_down = false;
            if (hit_actor->script.bank) {
                script_execute(hit_actor->script.bank, hit_actor->script.ptr, 0, 1, 0);
            }
        }
    }

    if (up_down) {
        PLAYER.pos.y = PLAYER.pos.y - ((up_down_dir - 1) * PLAYER.move_speed);
        up_down = false;
    }
    if (player_moving) {
        PLAYER.pos.x = PLAYER.pos.x - ((dir_anim - 2) * PLAYER.move_speed);
        player_moving = false;
    } 
    
}
