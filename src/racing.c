/* 
 * Tux Racer 
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "tuxracer.h"
#include "racing.h"
#include "audio.h"
#include "keyboard.h"
#include "keyframe.h"
#include "course_render.h"
#include "multiplayer.h"
#include "gl_util.h"
#include "fps.h"
#include "loop.h"
#include "render_util.h"
#include "view.h"
#include "tux.h"
#include "tux_shadow.h"
#include "phys_sim.h"
#include "part_sys.h"
#include "screenshot.h"
#include "fog.h"
#include "viewfrustum.h"
#include "track_marks.h"
#include "hud.h"
#include "hud_training.h"
#include "joystick.h"
#include "bonus.h"
#ifdef TARGET_OS_IPHONE
    #import "sharedGeneralFunctions.h"
#endif

/* Time constant for automatic steering centering (s) */
#define TURN_DECAY_TIME_CONSTANT 0.5

/* Maximum time over which jump can be charged */
#define MAX_JUMP_AMT 1.0

/* Time constant for automatic rolling centering (s) */
#define ROLL_DECAY_TIME_CONSTANT 0.2

#define JUMP_CHARGE_DECAY_TIME_CONSTANT 0.1

/* If too high off the ground, tux flaps instead of jumping */
#define JUMP_MAX_START_HEIGHT 0.30

/* factor limit to be considered as a left or a right turn. useful only for tricks */
#define TURN_FACTOR_LIMIT 0.1

#define TAP_EFFECT_LENGTH_TICKS 500

static bool_t right_turn;
static bool_t left_turn;
static bool_t trick_modifier;
static bool_t paddling;
static bool_t charging;
static bool_t braking;
static scalar_t charge_start_time;
static int last_terrain;

static bool_t touch_control=False;
static int last_paddle_tap=0;
static int last_brake_tap=0;
static int last_trick_tap=0;
static int charge_finger_index=-1;

static scalar_t joy_x;
static scalar_t joy_y;
static bool_t joy_left_turn = False;
static bool_t joy_right_turn = False;
static scalar_t joy_turn_fact = 0.0;
static bool_t joy_paddling = False;
static bool_t joy_braking = False;
static bool_t joy_tricks = False;
static bool_t joy_charging = False;


#ifdef FLYING_TIME_LIMIT

bool_t get_trick_modifier() {
    return trick_modifier;
}

/* ce init ne ser que pendant le tutorial, quand on souhaite faire revenir tux en arriere */
void racing_init_for_tutorial(point_t point) {
    player_data_t *plyr = get_player_data( local_player() );
    plyr->control.is_flying=False;
    plyr->control.fly_total_time=0;
    init_physical_simulation_at_point(plyr,point);
}

#endif

void racing_mouse_func(int button, int state, int finger_index, int absolute_x, int absolute_y)
{
	float width=getparam_x_resolution(), height=getparam_y_resolution();
	float x=absolute_x/width, y=absolute_y/height;

	touch_control=True;

	if (state==SDL_PRESSED)
	{
		if (x>0.33 && x<0.67)
		{
			if (state==SDL_PRESSED)
				set_game_mode(PAUSED);
		}
		else if (x<0.33)
		{
			if (y>0.5)
			{
				last_brake_tap=SDL_GetTicks();
			}
			else
			{
				charging=True;
				charge_finger_index=finger_index;
			}
		}
		else if (x>0.67)
		{
			if (y>0.5)
			{
				last_paddle_tap=SDL_GetTicks();
			}
			else
			{
				last_trick_tap=SDL_GetTicks();
			}
		}
	}
	else if (charging && charge_finger_index==finger_index)
	{
		charging=False;
		charge_finger_index=-1;
	}

}

void racing_joystick_func(double x, double y)
{
	joy_x=x;
	joy_y=y;
}

void racing_init(void) 
{
    player_data_t *plyr = get_player_data( local_player() );
    
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    if (SDL_GetNumTouchDevices()>0 || 1)
	{
		winsys_set_mouse_func(racing_mouse_func);
	}
	else
	{
		winsys_set_mouse_func(NULL);
	}
    winsys_set_motion_func( NULL );
    winsys_set_passive_motion_func( NULL );
	winsys_set_joystick_func( racing_joystick_func );

	winsys_reset_js_bindings();
	winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_A, getparam_jump_key()[0]);
	winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_B, getparam_trick_modifier_key()[0]);
	winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_X, getparam_quit_key()[0]);
        
    /* Initialize view */
    if ( getparam_view_mode() < 0 || 
        getparam_view_mode() >= NUM_VIEW_MODES ) 
    {
        setparam_view_mode( FOLLOW );
    }
    
    if (g_game.practicing) {
        set_view_mode( plyr, (view_mode_t)getparam_view_mode() );
    } else {
    /* en mode training il faut qu'on voie tux pour voir quand il paddle, quand il fait des tricks, etc... */
        set_view_mode( plyr, FOLLOW );
    }
    
    /* We need to reset controls here since callbacks won't have been
     called in paused mode. This results in duplication between this
     code and init_physical_simulation.  Oh well. */
    left_turn = right_turn = paddling = False;
    trick_modifier = False;
    charging = False;
    plyr->control.turn_fact = 0.0;
    plyr->control.turn_animation = 0.0;
    plyr->control.is_braking = False;
    plyr->control.is_paddling = False;
    plyr->control.jumping = False;
    plyr->control.jump_charging = False;
#ifdef FLYING_TIME_LIMIT
    plyr->control.is_flying=False;
    plyr->control.fly_total_time=0;
#endif
    
    /* Set last_terrain to a value not used below */
    last_terrain = 0;
    
    if ( g_game.prev_mode != PAUSED ) {
        init_physical_simulation();
    }
    
    last_terrain = 0;
    
    g_game.race_aborted = False;
    g_game.race_time_over = False;
    
    play_music( "racing" );
    /* play_sound( "start_race", 0 ); */
}

void calc_jump_amt( scalar_t time_step )
{
    player_data_t *plyr = get_player_data( local_player() );
    
    if ( plyr->control.jump_charging ) {
        plyr->control.jump_amt = min( 
                                     MAX_JUMP_AMT, g_game.time - charge_start_time );
    } else if ( plyr->control.jumping ) {
        plyr->control.jump_amt *= 
	    ( 1.0 - ( g_game.time - plyr->control.jump_start_time ) / 
         JUMP_FORCE_DURATION );
    } else {
        plyr->control.jump_amt = 0;
    }
}

void racing_loop( scalar_t time_step )
{
    int width, height;
    player_data_t *plyr = get_player_data( local_player() );
    bool_t airborne;
    vector_t dir;
    scalar_t speed;
    scalar_t terrain_weights[NumTerrains];
    int new_terrain = 0;
    int slide_volume;
	bool_t flip;
	bool_t roll;
    
    dir = plyr->vel;
    speed = normalize_vector(&dir);
    
    airborne = plyr->airborne;
    
    width = getparam_x_resolution();
    height = getparam_y_resolution();
    
    check_gl_error();
    
    new_frame_for_fps_calc();
    
    update_audio();
    
    clear_rendering_context();
    
    setup_fog();
    
#ifndef TARGET_OS_IPHONE
    /*
     * Joystick
     */
    if ( winsys_is_joystick_active() ) {
		joy_left_turn = False;
		joy_right_turn = False;
		joy_turn_fact = 0.0;
		joy_paddling = False;
		joy_braking = False;
		joy_tricks = False;
		joy_charging = False;

		if ( joy_x > 0.1 ) {
			joy_right_turn = True;
			joy_turn_fact = joy_x;
		} else if ( joy_x < -0.1 ) {
			joy_left_turn = True;
			joy_turn_fact = joy_x;
		}

		if ( !joy_braking ) {
			joy_braking = (bool_t) ( joy_y > 0.5 );
		}

		if ( !joy_paddling ) {
			joy_paddling = (bool_t) ( joy_y < -0.5 );
		}
    }
#endif
    /* Update braking */
    plyr->control.is_braking = (bool_t) ( braking || joy_braking );

	if (touch_control)
	{
		braking = (bool_t)((SDL_GetTicks()-last_brake_tap)<TAP_EFFECT_LENGTH_TICKS);
	}
    
    if ( airborne ) {
        new_terrain = (1<<NumTerrains);
        
        /*
         * Tricks
         */
		if (touch_control)
		{
			trick_modifier = (bool_t)((SDL_GetTicks()-last_trick_tap)<TAP_EFFECT_LENGTH_TICKS);
		}
        if ( trick_modifier || joy_tricks) {
            if (left_turn || joy_left_turn) {
                plyr->control.barrel_roll_left = True;
            }

            if (right_turn || joy_right_turn) {
                plyr->control.barrel_roll_right = True;
            }

            if (paddling || joy_paddling) {
                plyr->control.front_flip = True;
            }

            if (plyr->control.is_braking) {
                plyr->control.back_flip = True;
            }
            
            //Par défaut ca fait un front flip
            if(!plyr->control.front_flip && !plyr->control.back_flip &&
               !plyr->control.barrel_roll_right && !plyr->control.barrel_roll_left )
            {
                plyr->control.back_flip = True;
            }
#ifdef TARGET_OS_IPHONE
            TRDebugLog("tricks : %d",plyr->tricks);
#endif
        }
        
        
    } else {
        
        get_surface_type(plyr->pos.x, plyr->pos.z, terrain_weights);
        if (terrain_weights[Snow] > 0) {
            new_terrain |= (1<<Snow);
        }
        if (terrain_weights[Rock] > 0) {
            new_terrain |= (1<<Rock);
        } 
        if (terrain_weights[Ice] > 0) {
            new_terrain |= (1<<Ice);
        }
        
    }
    
    /*
     * Jumping
     */
    calc_jump_amt( time_step );
    
    if ( ( charging || joy_charging ) && 
        !plyr->control.jump_charging && !plyr->control.jumping ) 
    {
        plyr->control.jump_charging = True;
        charge_start_time = g_game.time;
    }
    
    if ( ( !charging && !joy_charging ) && plyr->control.jump_charging ) {
        plyr->control.jump_charging = False;
        plyr->control.begin_jump = True;
    }
    
#ifdef TARGET_OS_IPHONE
    /* 
     * Turning 
     */
    scalar_t iPhone_turn_fact=accelerometerTurnFact();
    
    /*left_turn and right_turn are informations useful for tricks*/
    if (iPhone_turn_fact>TURN_FACTOR_LIMIT) {
        left_turn=False;
        right_turn=True;
    }
    else if (iPhone_turn_fact<-TURN_FACTOR_LIMIT) {
        left_turn=True;
        right_turn=False;
    } else left_turn = right_turn = False;

    plyr->control.turn_fact = iPhone_turn_fact;
    plyr->control.turn_animation = iPhone_turn_fact;
#else
    if ( ( left_turn || joy_left_turn )  ^ (right_turn || joy_right_turn ) ) {
	bool_t turning_left = (bool_t) ( left_turn || joy_left_turn );

	if ( joy_left_turn || joy_right_turn ) {
	    plyr->control.turn_fact = joy_turn_fact;
	} else {
	    plyr->control.turn_fact = (turning_left?-1:1);
	}

	plyr->control.turn_animation += (turning_left?-1:1) *
	    0.15 * time_step / 0.05;
	plyr->control.turn_animation = 
	    min(1.0, max(-1.0, plyr->control.turn_animation));
    } else {
	plyr->control.turn_fact = 0;

	/* Decay turn animation */
	if ( time_step < ROLL_DECAY_TIME_CONSTANT ) {
	    plyr->control.turn_animation *= 
		1.0 - time_step/ROLL_DECAY_TIME_CONSTANT;
	} else {
	    plyr->control.turn_animation = 0.0;
	}
    }

#endif
    
    
    /*
     * Paddling
     */
	if (touch_control)
	{
		paddling = (bool_t)((SDL_GetTicks()-last_paddle_tap)<TAP_EFFECT_LENGTH_TICKS);
	}

    if ( ( paddling || joy_paddling ) && plyr->control.is_paddling == False ) {
        plyr->control.is_paddling = True;
        plyr->control.paddle_time = g_game.time;
    }
    
    /*
     * Play flying sound and add Flying time to plyr->control.fly_total_time)
     */
    if (new_terrain & (1<<NumTerrains)) {
        set_sound_volume("flying_sound", min(128, speed*2));
        
        if (!(last_terrain & (1<<NumTerrains))) {
            play_sound( "flying_sound", -1 );
#ifdef FLYING_TIME_LIMIT
            plyr->control.is_flying=True;
            plyr->control.fly_start_time = g_game.time;
#endif
        }
    } else {
        if (last_terrain & (1<<NumTerrains)) {
#ifdef FLYING_TIME_LIMIT
            plyr->control.is_flying=False;
            plyr->control.fly_end_time = g_game.time;
            if (plyr->control.fly_end_time-plyr->control.fly_start_time>FLYING_TIME_LIMIT) {
                plyr->control.fly_total_time += plyr->control.fly_end_time-plyr->control.fly_start_time;
            }
#endif
            halt_sound( "flying_sound" );
        }
    }
    
    /*
     * Play sliding sound
     */
    slide_volume = min( (((pow(plyr->control.turn_fact, 2)*128)) +
                         (plyr->control.is_braking?128:0) +
                         (plyr->control.jumping?128:0) +
                         20) *
                       (speed/10), 128 );
    if (new_terrain & (1<<Snow)) {
        set_sound_volume("snow_sound", slide_volume * terrain_weights[Snow]);
        if (!(last_terrain & (1<<Snow))) {
            play_sound( "snow_sound", -1 );
        }
    } else {
        if (last_terrain & (1<<Snow)) {
            halt_sound( "snow_sound" );
        }
    }
    if (new_terrain & (1<<Rock)) {
        //set_sound_volume("rock_sound", 128*pow((speed/2), 2) * terrain_weights[Rock]);
        
        int rockvol = slide_volume * 10 * terrain_weights[Rock];
        
        if (rockvol > 400)
            rockvol = 400;
        
        set_sound_volume("rock_sound", rockvol);
        if (!(last_terrain & (1<<Rock))) {
            play_sound( "rock_sound", -1 );
        }
    } else {
        if (last_terrain & (1<<Rock)) {
            halt_sound( "rock_sound" );
        }
    }
    if (new_terrain & (1<<Ice)) {
        set_sound_volume("ice_sound", slide_volume * terrain_weights[Ice]);
        if (!(last_terrain & (1<<Ice))) {
            play_sound( "ice_sound", -1 );
        }
    } else {
        if (last_terrain & (1<<Ice)) {
            halt_sound( "ice_sound" );
        }
    }
    last_terrain = new_terrain; 
    
    
    /*
     * gs
     */
    roll = plyr->control.barrel_roll_left || plyr->control.barrel_roll_right;
    flip = plyr->control.front_flip || plyr->control.back_flip;
    if(roll && plyr->control.barrel_roll_factor == 0)
    {
        if(flip)
        {
            if(plyr->control.barrel_roll_left)
                add_new_bonus("Hyper heavy Jump", get_score_for_trick(HYPER_HEAVY_JUMP));
            else
                add_new_bonus("Ray star hybrid Jump", get_score_for_trick(RAY_STAR_HYBRID_JUMP));
        }
        else
        {
            if(plyr->control.barrel_roll_left)
                add_new_bonus("Roll Left", get_score_for_trick(ROLL_LEFT));
            else
                add_new_bonus("Roll Right", get_score_for_trick(ROLL_RIGHT));
        }
    }

    if(flip && plyr->control.flip_factor == 0)
    {
        if(roll)
        {
            if(plyr->control.back_flip)
                add_new_bonus("Saturn ice Fever", get_score_for_trick(SATURN_ICE_FEVER));
            else
                add_new_bonus("Wild pinguin Show", get_score_for_trick(WILD_PINGUIN_SHOW));
        }
        else
        {
            if(plyr->control.back_flip)
                add_new_bonus("Back Flip", get_score_for_trick(BACK_FLIP));
            else
                add_new_bonus("Barlow's Wheel", get_score_for_trick(BARLOWS_WHEEL));
        }
    }

    if (roll) {
        plyr->tricks+=1;
        plyr->control.barrel_roll_factor += 
		( plyr->control.barrel_roll_left ? -1 : 1 ) * 0.05 * time_step / 0.05;
        if ( (plyr->control.barrel_roll_factor  > 1) ||
            (plyr->control.barrel_roll_factor  < -1) ) {
            plyr->control.barrel_roll_factor = 0;
            plyr->control.barrel_roll_left = plyr->control.barrel_roll_right = False;
        }
    }
    if (flip) {
        plyr->tricks+=1;
        plyr->control.flip_factor += 
		( plyr->control.back_flip ? -1 : 1 ) * 0.05 * time_step / 0.05;
        if ( (plyr->control.flip_factor  > 1) ||
            (plyr->control.flip_factor  < -1) ) {
            plyr->control.flip_factor = 0;
            plyr->control.front_flip = plyr->control.back_flip = False;
        }
    }

    update_player_pos( plyr, time_step );
	
    /* 
     * Track Marks
     */
    add_track_mark( plyr );
    
    
    update_view( plyr, time_step );
    
    setup_view_frustum( plyr, NEAR_CLIP_DIST, 
                       getparam_forward_clip_distance() );
    
    draw_sky(plyr->view.pos);
    
    draw_fog_plane();
    
    set_course_clipping( True );
    set_course_eye_point( plyr->view.pos );
    setup_course_lighting();
    render_course();
    draw_trees();
    
    if ( getparam_draw_particles() ) {
        update_particles( time_step );
        draw_particles( plyr );
    }
    
    draw_tux();
    
    draw_hud( plyr );
    
    draw_hud_training(plyr);
    
    reshape( width, height );
    
    winsys_swap_buffers();
    
    g_game.time += time_step;
} 

static void racing_term(void)
{
    halt_sound( "flying_sound" );
    halt_sound( "rock_sound" );
    halt_sound( "ice_sound" );
    halt_sound( "snow_sound" );
    break_track_marks();
}


START_KEYBOARD_CB( quit_racing_cb )
{
	touch_control=False;
    if ( release ) return;
    g_game.race_aborted = True;
    set_game_mode( GAME_OVER );
}
END_KEYBOARD_CB


START_KEYBOARD_CB( turn_left_cb )
{
	touch_control=False;
    left_turn = (bool_t) !release;
}
END_KEYBOARD_CB


START_KEYBOARD_CB( turn_right_cb )
{
	touch_control=False;
    right_turn = (bool_t) !release;
}
END_KEYBOARD_CB


START_KEYBOARD_CB( trick_modifier_cb )
{
	touch_control=False;
    trick_modifier = (bool_t) !release;
}
END_KEYBOARD_CB


START_KEYBOARD_CB( brake_cb )
{
	touch_control=False;
    braking = (bool_t) !release;
}
END_KEYBOARD_CB

START_KEYBOARD_CB( paddle_cb )
{
	touch_control=False;
    paddling = (bool_t) !release;
    if (paddling) plyr->control.is_accelerating = True; else  plyr->control.is_accelerating = False;
}
END_KEYBOARD_CB


START_KEYBOARD_CB( above_view_cb )
{
	touch_control=False;
    if ( release ) return;
    set_view_mode( plyr, ABOVE );
    setparam_view_mode( ABOVE );
}
END_KEYBOARD_CB

START_KEYBOARD_CB( follow_view_cb )
{
	touch_control=False;
    if ( release ) return;
    set_view_mode( plyr, FOLLOW );
    setparam_view_mode( FOLLOW );
}
END_KEYBOARD_CB

START_KEYBOARD_CB( behind_view_cb )
{
	touch_control=False;
    if ( release ) return;
    set_view_mode( plyr, BEHIND );
    setparam_view_mode( BEHIND );
}
END_KEYBOARD_CB

START_KEYBOARD_CB( screenshot_cb )
{
	touch_control=False;
    if ( release ) return;
    screenshot();
}
END_KEYBOARD_CB

START_KEYBOARD_CB( pause_cb )
{
	touch_control=False;
    if ( release ) return;
    g_game.race_paused;
    set_game_mode( PAUSED );
}
END_KEYBOARD_CB

START_KEYBOARD_CB( reset_cb )
{
	touch_control=False;
    if ( release ) return;
    set_game_mode( RESET );
}
END_KEYBOARD_CB

START_KEYBOARD_CB( jump_cb )
{
	touch_control=False;
    charging = (bool_t) !release;
}
END_KEYBOARD_CB

START_KEYBOARD_CB( racing_default_cb )
{
	if (key==SDLK_AC_BACK)
	{
		if ( release ) return;
		g_game.race_aborted = True;
		set_game_mode( GAME_OVER );
	}
}
END_KEYBOARD_CB

void racing_register()
{
    int status = 0;

    status |= add_keymap_entry( RACING, DEFAULT_CALLBACK, 
                               NULL, NULL, racing_default_cb );

    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "escape", getparam_quit_key, quit_racing_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "j", getparam_turn_left_key, turn_left_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "l", getparam_turn_right_key, turn_right_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "d", getparam_trick_modifier_key, 
                               trick_modifier_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "space", getparam_brake_key, brake_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "k", getparam_paddle_key, paddle_cb );
    
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "1", getparam_behind_view_key, 
                               behind_view_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "2", getparam_follow_view_key, 
                               follow_view_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "3", getparam_above_view_key, above_view_cb );
    
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "=", getparam_screenshot_key, screenshot_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "p", getparam_pause_key, pause_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "r", getparam_reset_key, reset_cb );
    status |= add_keymap_entry( RACING, CONFIGURABLE_KEY, 
                               "i", getparam_jump_key, jump_cb );
    
    check_assertion( status == 0, "out of keymap entries" );
    
    register_loop_funcs( RACING, racing_init, racing_loop, racing_term );
}
