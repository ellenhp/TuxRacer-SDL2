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
#include "viewfrustum.h"
#include "hud.h"
#include "game_logic_util.h"
#include "fonts.h"
#include "ui_mgr.h"
#include "ui_snow.h"
#include "ui_theme.h"
#include "joystick.h"
#include "part_sys.h"
#ifdef ASYNC_LOADING
    #include <pthread.h>
#endif

#define NEXT_MODE RACING

static char *loaded_course = NULL;
static race_conditions_t loaded_conditions = (race_conditions_t)-1;

#ifdef ASYNC_LOADING
//En fait, la fonction loading_loop n'est pas une vrai fonction Loop, elle n'est executée qu'une fois, et comme ce n'est qu'une fois qu'on en sort que
//L'écran est redessiné, il faut la faire tourner au moins une fois sans rien loader pour que l'écran loading aparaisse, sinon il apparait quand tout est déjà chargé.
static int passInLoop;
#endif

/*---------------------------------------------------------------------------*/
/*! 
 Draws the text for the loading screen
 \author  jfpatry
*/
void draw_loading_text( void )
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    int x_org, y_org;
    const char *string;
    int string_w, asc, desc;
    font_t *font;
    
    x_org = w/2.0;
    y_org = h/2.0;
    
    if ( !get_font_binding( "loading", &font ) ) {
        print_warning( IMPORTANT_WARNING,
                      "Couldn't get font for binding loading" );
    } else {
        string = Localize("Loading, Please Wait...","");
        get_font_metrics( font, string, &string_w, &asc, &desc );
        
        glPushMatrix();
        {
            glTranslatef( w/2.0 - string_w/2.0,
                         h/2.0 - desc-30, 
                         0 );
            bind_font_texture( font );
            draw_string( font, string );
        }
        glPopMatrix();
    }
}

void loading_init(void) 
{
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( ui_event_mouse_func );
    winsys_set_motion_func( ui_event_motion_func );
    winsys_set_passive_motion_func( NULL );

#ifdef ASYNC_LOADING
    passInLoop = 0;
    
    draw_loading_text();
    
    reshape( getparam_x_resolution(), getparam_y_resolution() );
    
    stop_music();
#else
	play_music( "loading" );
#endif
}

#ifdef ASYNC_LOADING

static pthread_t loading_thread = 0;
static pthread_mutex_t load_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool_t loading_done = False;

void * load_course_thread(void * unused)
{
#ifdef TR_DEBUG_MODE
    uint64_t load_start = udate();
#endif
    /* Load the course */
    load_course_core( g_game.race.course );
    
    pthread_mutex_lock(&load_mutex);
    loading_done = True;
    pthread_mutex_unlock(&load_mutex);

#ifdef TR_DEBUG_MODE
    TRDebugLog("(loading thread) Loading took %dms\n", (int32_t)(((int64_t)udate() - (int64_t)load_start) / 1000000));
#endif

    return NULL;
}

#ifdef TR_DEBUG_MODE
uint64_t abs_load_start = 0;
#endif

#endif // ASYNC_LOADING

void loading_loop( scalar_t time_step )
{
    int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();

    check_gl_error();
    
    update_audio();
    
    clear_rendering_context();
    
    set_gl_options( GUI );
    
    ui_setup_display();
    
    if (getparam_ui_snow()) {
        update_ui_snow( time_step, g_game.race.windy );
        draw_ui_snow();
    }
    
    ui_draw_menu_decorations(True);
    
    draw_loading_text();
    
    reshape( width, height );
    
    winsys_swap_buffers();

#ifdef ASYNC_LOADING
    if(passInLoop++ < 2) return;

    if ( loaded_course == NULL ||
        loaded_course != g_game.race.course ||
        loaded_conditions != g_game.race.conditions ) 
    {
		bool_t done;

        if(!loading_thread) {
            loading_done = False;
#ifdef TR_DEBUG_MODE
            abs_load_start = udate();
#endif
            preload_course(g_game.race.course);
            winsys_set_high_framerate(False);
            pthread_create(&loading_thread, NULL, load_course_thread, NULL);
        }
        pthread_mutex_lock(&load_mutex);
        done = loading_done;
        pthread_mutex_unlock(&load_mutex);
        if(done) {
#ifdef ASYNC_LOADING
            pthread_join(loading_thread, NULL);
#endif
            loading_thread = 0;

#ifdef TR_DEBUG_MODE
            TRDebugLog("(main thread) loading thread took %dms\n", (int32_t)(((int64_t)udate() - (int64_t)abs_load_start) / 1000000));
#endif

            postload_course(g_game.race.course);

            loaded_course = g_game.race.course;
            loaded_conditions = g_game.race.conditions;
            
            set_course_mirroring( g_game.race.mirrored );
    
            winsys_set_high_framerate(True);

            /* We're done here, enter INTRO mode */
            set_game_mode(INTRO);
#ifdef TR_DEBUG_MODE
            TRDebugLog("Total loading took %dms\n", (int32_t)(((int64_t)udate() - (int64_t)abs_load_start) / 1000000));
#endif

        }
    } else {
        winsys_set_high_framerate(True);

        set_course_mirroring( g_game.race.mirrored );

        set_game_mode(INTRO);
    }
#else
    if ( loaded_course == NULL ||
	 loaded_course != g_game.race.course ||
	 loaded_conditions != g_game.race.conditions ) 
    {
	/* Load the course */
	load_course( g_game.race.course );

	loaded_course = g_game.race.course;
	loaded_conditions = g_game.race.conditions;
    }

    set_course_mirroring( g_game.race.mirrored );

    /* We're done here, enter INTRO mode */
    set_game_mode( INTRO );
#endif // ASYNC_LOADING

} 

void loading_register()
{
    register_loop_funcs( LOADING, loading_init, loading_loop, NULL );
}


