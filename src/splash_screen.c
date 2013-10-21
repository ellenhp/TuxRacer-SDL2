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
#include "keyboard.h"
#include "gl_util.h"
#include "fps.h"
#include "loop.h"
#include "render_util.h"
#include "textures.h"
#include "multiplayer.h"
#include "ui_mgr.h"
#include "ui_snow.h"
#ifdef TARGET_OS_IPHONE
    #include "sharedGeneralFunctions.h"
#endif

#define COORD_OFFSET_AMT -0.5
#define SPLASH_DISPLAY_TICKS 1500

static const colour_t background_colour = { 0.48, 0.63, 0.90, 1.0 };

static char* logo_binding = "logo";

static int splash_start_ticks=0;

static void goto_next_mode()
{
    int i;
    set_game_mode( GAME_TYPE_SELECT );

    /* 
     * Free textures
     */
	unbind_texture( logo_binding );
    flush_textures();
    winsys_post_redisplay();
}

void splash_screen_js_func(int btn)
{
	goto_next_mode();
}

static void splash_screen_mouse_func( int button, int state, int finger_index, int x, int y )
{
#ifdef TARGET_OS_IPHONE
    if (state==WS_MOUSE_DOWN) {
#endif
        goto_next_mode();
#ifdef TARGET_OS_IPHONE
    }
#endif
}

void splash_screen_init(void) 
{
    init_ui_snow();

    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( splash_screen_mouse_func );
    winsys_set_motion_func( ui_event_motion_func );
    winsys_set_passive_motion_func( ui_event_motion_func );

	winsys_set_joystick_button_func( splash_screen_js_func );

#ifndef TARGET_OS_IPHONE
    play_music( "splash_screen" );
#endif

    reshape( getparam_x_resolution(), getparam_y_resolution() );

	splash_start_ticks=SDL_GetTicks();
    
#ifdef TARGET_OS_IPHONE
    // Skip the splash screen directly
    goto_next_mode();
#endif
}


static void draw_logo()
{
	GLuint texid;
	point2d_t ll, ur;
	GLint w, h;
	int i;

	glEnable( GL_TEXTURE_2D );

	if ( ! get_texture_binding( logo_binding, &texid ) ) {
		return;
	}

	glColor4f( 1.0, 1.0, 1.0, 1.0 );

	glBindTexture( GL_TEXTURE_2D, texid );

	h = getparam_y_resolution()/2; //scale to screen size
	w = h*2;

	ll.x = getparam_x_resolution()/2 - w/2;
	ll.y = getparam_y_resolution()/2 - h/2;
	ur.x = ll.x + w;
	ur.y = ll.y + h;

	{
		GLfloat texcoords[]={
			0, 0,
			0, 1,
			1, 1,
			1, 0};

		GLfloat vertices[]={
			ll.x, ll.y, 0,
			ll.x, ur.y, 0,
			ur.x, ur.y, 0,
			ur.x, ll.y, 0};

		GLubyte indices[] = {0, 1, 2, 2, 3, 0};

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}


void splash_screen_loop( scalar_t time_step )
{
	if (SDL_GetTicks()-splash_start_ticks>SPLASH_DISPLAY_TICKS)
	{
	    goto_next_mode();
	}

    check_gl_error();

    update_audio();

    clear_rendering_context();

    set_gl_options( GUI );

    ui_setup_display();

    if (getparam_ui_snow()) {
		update_ui_snow( time_step, False );
		draw_ui_snow();
    }

    draw_logo();

    ui_draw();

//    reshape( getparam_x_resolution(), getparam_y_resolution() );

    winsys_swap_buffers();
} 

START_KEYBOARD_CB( splash_screen_cb )
{
#ifndef TARGET_OS_IPHONE
    goto_next_mode();
#endif
}
END_KEYBOARD_CB

START_KEYBOARD_CB( toggle_snow )
{
    if ( release ) return;

    setparam_ui_snow( (bool_t) !getparam_ui_snow() );
}
END_KEYBOARD_CB

void splash_screen_register()
{
    int status = 0;

    status |= add_keymap_entry( SPLASH, DEFAULT_CALLBACK, NULL, NULL, 
				splash_screen_cb );

    status |= add_keymap_entry( ALL_MODES, FIXED_KEY, "tab", NULL, 
				toggle_snow );

    check_assertion( status == 0, "out of keymap entries" );

    register_loop_funcs( SPLASH, splash_screen_init, splash_screen_loop, 
			 NULL );

}

