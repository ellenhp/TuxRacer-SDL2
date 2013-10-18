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
#include "game_type_select.h"
#include "ui_mgr.h"
#include "ui_theme.h"
#include "button.h"
#include "loop.h"
#include "render_util.h"
#include "audio.h"
#include "gl_util.h"
#include "keyboard.h"
#include "multiplayer.h"
#include "ui_snow.h"
#include "joystick.h"
#include "event_select.h"
#include "winsys.h"
#include "gui_abstraction.h"
#include "gui_mgr.h"

#ifdef TARGET_OS_IPHONE
    #include "sharedGeneralFunctions.h"
#endif

widget_t* enter_event_btn = NULL;
widget_t* practice_btn = NULL;
widget_t* credits_btn = NULL;
widget_t* pref_btn = NULL;
widget_t* quit_btn = NULL;

//The training mode of Tux Racer World challenge has been binded to the event mode of tuxracer
void enter_event_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
    g_game.current_event = NULL;
    g_game.current_cup = NULL;
    g_game.current_race = -1;
    g_game.practicing = False;
    
    zappe_event_screen(NULL, NULL);
    
    ui_set_dirty();
}

//wich is in this version the "world challenge click callback
void practice_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
#ifdef TARGET_OS_IPHONE
    //Only registered players can go in this mode
    /* if (playerRegistered()) { */
#endif
        g_game.current_event = NULL;
        g_game.current_cup = NULL;
        g_game.current_race = -1;
        g_game.practicing = True;
#ifdef TARGET_OS_IPHONE
        //usefull for score saving
        g_game.race.name=NULL;
#endif
#ifdef SPEED_MODE
        set_game_mode( RACING_MODE_SELECT );
#else
        set_game_mode( RACE_SELECT );
#endif
        
        ui_set_dirty();
#ifdef TARGET_OS_IPHONE
    /* } else {
        alertRegisterNeeded();
    } */
#endif
}

void credits_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
    set_game_mode( CREDITS );

    ui_set_dirty();
}

void pref_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
    set_game_mode( PREFS );

    ui_set_dirty();
}

void quit_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
    winsys_exit( 0 );
}

static void game_type_select_init(void)
{
    point2d_t dummy_pos = {0, 0};

    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( GameMenu_mouse_func );
    winsys_set_motion_func( GameMenu_motion_func );
    winsys_set_passive_motion_func( GameMenu_motion_func );
	winsys_set_joystick_func( GameMenu_joystick_func );
	winsys_set_joystick_button_func( GameMenu_joystick_button_func );

	setup_gui();

	gui_add_widget(practice_btn=create_button("Play", practice_click_cb), NULL);
	gui_add_widget(enter_event_btn=create_button("Tutorial", enter_event_click_cb), NULL);
	gui_add_widget(pref_btn=create_button("Settings", pref_click_cb), NULL);
	gui_add_widget(credits_btn=create_button("Credits", credits_click_cb), NULL);
	gui_add_widget(quit_btn=create_button("Quit", quit_click_cb), NULL);

	winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_B, SDLK_ESCAPE);

    play_music( "start_screen" );

	reshape( getparam_x_resolution(), getparam_y_resolution() );

}

static void game_type_select_loop( scalar_t time_step )
{
    check_gl_error();

    update_audio();

    set_gl_options( GUI );

    clear_rendering_context();

    ui_setup_display();

    if (getparam_ui_snow()) {
	update_ui_snow( time_step, False );
	draw_ui_snow();
    }

    ui_draw_menu_decorations();

	GameMenu_draw();

	ui_draw_cursor();

    reshape( getparam_x_resolution(), getparam_y_resolution() );

    winsys_swap_buffers();
}

static void game_type_select_term(void)
{
    reset_gui();
}

START_KEYBOARD_CB( game_type_select_cb )
{
    if (release) return;

    if ( !special ) {
	key = (int) tolower( (char) key );
	}

	switch( key ) {
	case 'q':
	case 27: /* Esc */
	case SDLK_AC_BACK:
	    winsys_exit(0);
	    break;
	case 'e':
	    if ( enter_event_btn ) {
		GameMenu_simulate_click( enter_event_btn );
	    }
	    break;
	case 'p':
	    if ( practice_btn ) {
		GameMenu_simulate_click( practice_btn );
	    }
	    break;
	case 'c':
	    if ( credits_btn ) {
		GameMenu_simulate_click( credits_btn );
	    }
	    break;
	}

	GameMenu_keypress(key);
    
	winsys_post_redisplay();
}
END_KEYBOARD_CB

void game_type_select_register()
{
    int status = 0;

    status |=
	add_keymap_entry( GAME_TYPE_SELECT,
			  DEFAULT_CALLBACK, NULL, NULL, game_type_select_cb );
    register_loop_funcs( GAME_TYPE_SELECT, 
			 game_type_select_init,
			 game_type_select_loop,
			 game_type_select_term );

}


/* EOF */
