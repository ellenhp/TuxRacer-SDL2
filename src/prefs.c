

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
#include "fog.h"
#include "viewfrustum.h"
#include "hud.h"
#include "game_logic_util.h"
#include "fonts.h"
#include "ui_mgr.h"
#include "ui_theme.h"
#include "ui_snow.h"
#include "gui_abstraction.h"
#include "gui_mgr.h"
#include "gui_button.h"
#include "gui_slider.h"
#include "joystick.h"

static void prefs_init(void) 
{
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( TRGui_mouse_func );
    winsys_set_motion_func( TRGui_motion_func );
    winsys_set_passive_motion_func( TRGui_motion_func );

	TRGui_init();
	setup_gui();

    play_music( "start_screen" );
}

static void prefs_loop( scalar_t time_step )
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
        update_ui_snow( time_step, False );
        draw_ui_snow();
    }

	gui_draw();
    
    ui_draw_cursor();
    
    reshape( width, height );
    
    winsys_swap_buffers();
} 

START_KEYBOARD_CB( prefs_key_cb )
{
	if (release) return;

    if ( !special )
	{
		key = (int) tolower( (char) key );

		switch( key )
		{
		case 'q':
		case 27: /* Esc */
			break;
		}
    }
}
END_KEYBOARD_CB

void prefs_register()
{
    int status = 0;
    
    status |= add_keymap_entry( PREFS, 
                               DEFAULT_CALLBACK, 
                               NULL, NULL, prefs_key_cb );
    
    check_assertion( status == 0, "out of keymap entries" );
    
    register_loop_funcs( PREFS, prefs_init, prefs_loop, NULL );
}


