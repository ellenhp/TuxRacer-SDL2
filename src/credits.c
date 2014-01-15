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
#include "game_logic_util.h"
#include "fonts.h"
#include "ui_mgr.h"
#include "ui_theme.h"
#include "ui_snow.h"
#include "joystick.h"
#include "gui_abstraction.h"

#define CREDITS_MAX_Y -140
#define CREDITS_MIN_Y 64

#define CREDITS_SPEED 40
#define CREDITS_JOYSTICK_SPEED 150

double joystick_y;

typedef struct {
    char *binding;
    char *text;
} credit_line_t;

static credit_line_t credit_lines[] = 
{
    { "credits_text", "" },
    { "credits_h1", "Tux Racer" }, //a real name will come later
    { "credits_text_small", "" },
    { "credits_text_small", "This program is free software;" },
    { "credits_text_small", "you can redistribute it and/or" },
    { "credits_text_small", "modify it under the terms of the" },
    { "credits_text_small", "GNU General Public License" },
    { "credits_text_small", "" },
    { "credits_text", "Ported from" },
    { "credits_text", "the open source project" },
    { "credits_text", "Tux Racer 4iOS" },
    { "credits_text", "" },
    { "credits_h2", "Development Team:" },
    { "credits_text_small", "(Alphabetical Order)" },
    { "credits_text", "Lennie Araki" },
    { "credits_text", "Nolan Poe" },
    { "credits_text", "" },
    { "credits_h2", "Original Tux Racer" },
    { "credits_h2", "Core Development Team:" },
    { "credits_text_small", "(Alphabetical Order)" },
    { "credits_text", "Patrick \"Pog\" Gilhuly" },
    { "credits_text", "Eric \"Monster\" Hall" },
    { "credits_text", "Rick Knowles" },
    { "credits_text", "Vincent Ma" },
    { "credits_text", "Jasmin Patry" },
    { "credits_text", "Mark Riddell" },
    { "credits_text", "" },
    { "credits_h2", "Tux Racer 4iOS" },
    { "credits_h2", "Core Development Team:" },
    { "credits_text_small", "(Alphabetical Order)" },
    { "credits_text", "Emmanuel de Roux" },
    { "credits_text", "Felix Jankowski" },  
    { "credits_text", "" },
    { "credits_h2", "Source available at:" },
    { "credits_text", "https://github.com/nopoe/TuxRacer-SDL2" },
};

static scalar_t y_offset = 0;


/*---------------------------------------------------------------------------*/
/*! 
 Returns to the game type select screen
 \author  jfpatry
 \date    Created:  2000-09-27
 \date    Modified: 2000-09-27
 */
static void go_back() 
{
    set_game_mode( GAME_TYPE_SELECT );
	winsys_set_joystick_button_func(NULL);
    winsys_post_redisplay();
}


/*---------------------------------------------------------------------------*/
/*! 
 mouse callback
 \author  jfpatry
 \date    Created:  2000-09-27
 \date    Modified: 2000-09-27
 */
void mouse_cb( int button, int state, int finger_index, int x, int y )
{
    if ( state == WS_MOUSE_DOWN ) {
        go_back();
    }
}

/*---------------------------------------------------------------------------*/
/*! 
 Joystick axis event callback
 \author  nopoe
 \date    Created:  2013-10-08
 \date    Modified: 2013-10-08
 */
void credits_joystick_func(double x, double y)
{
	joystick_y=y;
}

/*---------------------------------------------------------------------------*/
/*! 
 Joystick button event callback
 \author  nopoe
 \date    Created:  2013-10-08
 \date    Modified: 2013-10-08
 */
void credits_joystick_button_func(int button)
{
	go_back();
}

/*---------------------------------------------------------------------------*/
/*! 
 Scrolls the credits text up the screen.
 \author  jfpatry
 \date    Created:  2000-09-27
 \date    Modified: 2000-09-27
 */
static void draw_credits_text( scalar_t time_step )
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    font_t* font;
    coord_t text_coord;
    int i, string_w, asc, desc;
    scalar_t y;
    
    y_offset += time_step * CREDITS_SPEED + time_step*joystick_y*CREDITS_JOYSTICK_SPEED;
	if (y_offset<0)
	{
		y_offset=0;
	}
    y = CREDITS_MIN_Y+y_offset;
    
    text_coord.x=w/2;
    text_coord.y=y;
    text_coord.x_coord_type=ABSOLUTE_COORD;
    text_coord.y_coord_type=ABSOLUTE_COORD;
    text_coord.x_just=text_coord.y_just=CENTER_JUST;
    
    for (i=0; i<sizeof( credit_lines ) / sizeof( credit_lines[0] ); i++) {
        credit_line_t line = credit_lines[i];
        
        if ( !get_font_binding( line.binding, &font ) ) {
            print_warning( IMPORTANT_WARNING,
                          "Couldn't get font for binding %s",
                          line.binding );
        } else {
            get_font_metrics( font, line.text, &string_w, &asc, &desc );
            
            text_coord.y -= asc+desc;

            GameMenu_draw_text( line.text, 0, text_coord, line.binding );
        }
    }
    
    if ( text_coord.y > h+CREDITS_MAX_Y ) {
        go_back();
    }
}

static void credits_init(void) 
{
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( mouse_cb );
    winsys_set_motion_func( ui_event_motion_func );
    winsys_set_passive_motion_func( ui_event_motion_func );
	winsys_set_joystick_func(credits_joystick_func);
	winsys_set_joystick_button_func(credits_joystick_button_func);
    
    y_offset = 0;
    
    play_music( "credits_screen" );
}

static void credits_loop( scalar_t time_step )
{
    int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();
    
    check_gl_error();
    
    update_audio();
    
    clear_rendering_context();
    
    set_gl_options( GUI );
    
    ui_setup_display();
    
    draw_credits_text( time_step );
    
    if (getparam_ui_snow()) {
        update_ui_snow( time_step, False );
        draw_ui_snow();
    }
    
    ui_draw();
    
    reshape( width, height );
    
    winsys_swap_buffers();
} 

START_KEYBOARD_CB( credits_key_cb )
{
    if ( !release ) {
        go_back();
    }
}
END_KEYBOARD_CB

void credits_register()
{
    int status = 0;
    
    status |= add_keymap_entry( CREDITS, 
                               DEFAULT_CALLBACK, 
                               NULL, NULL, credits_key_cb );
    
    check_assertion( status == 0, "out of keymap entries" );
    
    register_loop_funcs( CREDITS, credits_init, credits_loop, NULL );
}


