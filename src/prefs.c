

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

#define VOLUME_TOTAL_TICKS 10

#define MUSIC_VOLUME_STRING "Music Volume: %d"
#define SOUND_VOLUME_STRING "Effects Volume: %d"

widget_t* music_volume_slider=NULL;
widget_t* sound_volume_slider=NULL;

widget_t* back_btn=NULL;

int music_volume_ticks=0;
int sound_volume_ticks=0;

void update_volume()
{
	char* str;

	setparam_music_volume(music_volume_ticks);
	str=(char*)malloc(strlen(MUSIC_VOLUME_STRING)+1);
	sprintf(str, MUSIC_VOLUME_STRING, music_volume_ticks);
	widget_set_text(music_volume_slider, str);

	setparam_sound_volume(sound_volume_ticks);
	str=(char*)malloc(strlen(SOUND_VOLUME_STRING)+1);
	sprintf(str, SOUND_VOLUME_STRING, sound_volume_ticks);
	widget_set_text(sound_volume_slider, str);

	free(str);

	write_config_file();
}

void music_volume_down(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
	music_volume_ticks--;
	music_volume_ticks+=VOLUME_TOTAL_TICKS+1;
	music_volume_ticks%=VOLUME_TOTAL_TICKS+1;
	//modulus of negative number returns negative and negative volume is bad. this is kind of a hack, but it works

	update_volume();
}

void music_volume_up(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
	music_volume_ticks++;
	music_volume_ticks%=VOLUME_TOTAL_TICKS+1;

	update_volume();
}

void sound_volume_down(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
	sound_volume_ticks--;
	sound_volume_ticks+=VOLUME_TOTAL_TICKS+1;
	sound_volume_ticks%=VOLUME_TOTAL_TICKS+1;
	//modulus of negative number returns negative and negative volume is bad. this is kind of a hack, but it works

	update_volume();
}

void sound_volume_up(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
	sound_volume_ticks++;
	sound_volume_ticks%=VOLUME_TOTAL_TICKS+1;

	update_volume();
}

void back_click(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb)
{
	set_game_mode( GAME_TYPE_SELECT );
	reset_gui();
}

static void prefs_init(void) 
{
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( GameMenu_mouse_func );
    winsys_set_motion_func( GameMenu_motion_func );
    winsys_set_passive_motion_func( GameMenu_motion_func );

	GameMenu_init();
	setup_gui();

	gui_add_widget(back_btn=create_button("Back", back_click), NULL);

	music_volume_slider=create_slider("", music_volume_down, music_volume_up);
	gui_add_widget(music_volume_slider, NULL);

	sound_volume_slider=create_slider("", sound_volume_down, sound_volume_up);
	gui_add_widget(sound_volume_slider, NULL);

	//this is the result of some algebra. Initializes the volume slider values
	music_volume_ticks=getparam_music_volume();
	sound_volume_ticks=getparam_sound_volume();

	update_volume();

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

    GameMenu_keypress(key);
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


