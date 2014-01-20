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
#include "audio.h"
#include "game_config.h"
#include "multiplayer.h"
#include "gl_util.h"
#include "fps.h"
#include "render_util.h"
#include "phys_sim.h"
#include "view.h"
#include "course_render.h"
#include "tux.h"
#include "tux_shadow.h"
#include "keyboard.h"
#include "loop.h"
#include "hud.h"
#include "hud_training.h"
#include "part_sys.h"
#include "game_logic_util.h"
#include "fonts.h"
#include "ui_mgr.h"
#include "joystick.h"
#include "button.h"
#include "platform.h"
#include "gui_abstraction.h"
#include "gui_button.h"

#define NEXT_MODE RACING

unsigned int pause_start;
unsigned int pause_min_ticks;

static widget_t* play_button=NULL;
static widget_t* back_button=NULL;

void end_game()
{
    g_game.race_aborted=True;
    g_game.race_paused=False;
    set_game_mode(RACE_SELECT);
}
void come_back_to_game(void) {
	if (SDL_GetTicks()-pause_start<pause_min_ticks)
		return;
	pause_min_ticks=0;
	if (pause_is_for_long_tutorial_explanation)
	{
		training_resume_from_tutorial_explanation();
	}
    g_game.race_paused=False;
    set_game_mode( NEXT_MODE );
    winsys_post_redisplay();
}

void force_pause_for_ticks(int ticks)
{
	pause_min_ticks=(unsigned int)ticks;
}

static void mouse_cb( int button, int state, int finger_index, int x, int y )
{
	if (state==SDL_PRESSED)
	{
		come_back_to_game();
	}
}

void paused_joystick_button_func(int button)
{
	come_back_to_game();
}

void paused_continue_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
    come_back_to_game();
}

void paused_back_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
    end_game();
}

/*---------------------------------------------------------------------------*/
/*! 
  Draws the text for the paused screen
  \author  jfpatry
  \date    Created:  2000-09-26
  \date    Modified: 2000-09-26
*/
void draw_paused_text( void )
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    int x_org, y_org;
    int box_width, box_height;
    const char *string;
    int string_w, asc, desc;
    font_t *font;

    box_width = 200;
    box_height = 300;

    x_org = w/2.0 - box_width/2.0;
    y_org = h/2.0 - box_height/2.0;   

    if ( !get_font_binding( "paused", &font ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get font for binding paused" );
    } else {
	string = Localize("Paused","");

	get_font_metrics( font, string, &string_w, &asc, &desc );
	
        /*
	glPushMatrix();
	{
	    glTranslatef( x_org + box_width/2.0 - string_w/2.0,
			  y_org + box_height/2.0, 
			  0 );
	    bind_font_texture( font );
	    draw_string( font, string );
	}
	glPopMatrix();
         */
    }
}

void paused_init(void) 
{
    point2d_t dummy_pos = {0, 0};
	coord_t button_coord;

    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( GameMenu_mouse_func );
    winsys_set_motion_func( GameMenu_motion_func );
    winsys_set_passive_motion_func( GameMenu_motion_func );
	winsys_set_joystick_func( NULL );
	winsys_set_joystick_button_func( NULL );
    
	winsys_reset_js_bindings();
	winsys_add_js_axis_bindings();
	winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_A, SDLK_RETURN);
	winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_B, SDLK_ESCAPE);
    
    GameMenu_init();
	setup_gui();

    button_coord.x=0.37;
	button_coord.y=0.13;
	button_coord.x_just=button_coord.y_just=CENTER_JUST;
    button_coord.x_coord_type=button_coord.y_coord_type=NORMALIZED_COORD;
    if (winsys_is_controller_active())
    {
        gui_add_widget(back_button=create_label(get_abort_text(), paused_back_cb), &button_coord);
    }
    else
    {
        gui_add_widget(back_button=create_button(get_abort_text(), paused_back_cb), &button_coord);
    }
    
	button_coord.x=0.63;
    if (winsys_is_controller_active())
    {
        gui_add_widget(play_button=create_label(get_continue_text(), paused_continue_cb), &button_coord);
    }
    else
    {
        gui_add_widget(play_button=create_button(get_continue_text(), paused_continue_cb), &button_coord);
    }
    
	g_game.race_paused=True;

	pause_start=SDL_GetTicks();

    play_music( "paused" );
}

void paused_loop( scalar_t time_step )
{
    player_data_t *plyr = get_player_data( local_player() );
    int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();

    check_gl_error();
    
    new_frame_for_fps_calc();

    update_audio();

    clear_rendering_context();

    use_generic_program();
    
    update_player_pos( plyr, 0 );
    update_view( plyr, 0 );

    util_setup_projection(NEAR_CLIP_DIST, getparam_forward_clip_distance());
    
    set_course_clipping( True );
    set_course_eye_point( plyr->view.pos );
    
    use_terrain_program();
    
    render_course();
    
    use_generic_program();
    
    if ( getparam_draw_particles() ) {
	draw_particles( plyr );
    }

    draw_tux();

    set_gl_options( GUI );

    ui_setup_display();
    
    if (!pause_is_for_long_tutorial_explanation())
	{
        draw_paused_text();
        GameMenu_draw();
    }
    
    use_hud_program();
    
    draw_hud( plyr );
    
    draw_hud_training( plyr );

    use_generic_program();
    
    draw_sky( plyr->view.pos );
    
    draw_trees();
    
    reshape( width, height );

    winsys_swap_buffers();
} 

START_KEYBOARD_CB( paused_cb )
{
    if ( release ) {
        return;
    }
    switch (key) {
        case SDLK_AC_BACK:
        case 27: /* Esc */
            end_game();
            break;
        case 13: /* Enter */
            come_back_to_game();
            break;
    }
}
END_KEYBOARD_CB

void paused_register()
{
    int status = 0;

    status |= add_keymap_entry( PAUSED, 
				DEFAULT_CALLBACK, NULL, NULL, paused_cb );

    check_assertion( status == 0, "out of keymap entries" );

    register_loop_funcs( PAUSED, paused_init, paused_loop, NULL );
}
