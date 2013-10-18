

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

typedef struct graphics_options_t
{
	int course_detail_level;
	int forward_clip_distance;
	int tree_detail_distance;
	bool_t terrain_blending;
	bool_t perfect_terrain_blending;
	bool_t terrain_envmap;
	int tux_sphere_divisions;
	bool_t draw_particles;
	bool_t track_marks;
	char* name;
} graphics_options_t;

#define NUM_GRAPHICS_OPTIONS 3
graphics_options_t graphics_options[]={
	{5, 30, 0, False, False, False, 3, False, False, "Graphics Preset: Low"},
	{10, 70, 20, True, False, True, 16, True, False, "Graphics Preset: Medium"},
	{20, 300, 300, True, True, True, 64, True, True, "Graphics Preset: High"},
};

widget_t* back_btn=NULL;

widget_t* music_volume_slider=NULL;
widget_t* sound_volume_slider=NULL;

widget_t* graphics_slider=NULL;

#define NUM_VIEW_OPTIONS 4
widget_t* view_slider=NULL;

widget_t* fps_btn=NULL;

int music_volume_ticks=0;
int sound_volume_ticks=0;

int graphics_ticks=0;

int view_mode=0;

bool_t display_fps;

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

void update_graphics()
{
	graphics_options_t options=graphics_options[graphics_ticks];
	widget_set_text(graphics_slider, options.name);
	
	//apply tick param to remember setting for next start
	setparam_graphics_slider_tick(graphics_ticks);

	//apply settings
	setparam_course_detail_level(options.course_detail_level);
	setparam_forward_clip_distance(options.forward_clip_distance);
	setparam_tree_detail_distance(options.tree_detail_distance);
	setparam_terrain_blending(options.terrain_blending);
	setparam_perfect_terrain_blending(options.perfect_terrain_blending);
	setparam_terrain_envmap(options.terrain_envmap);
	setparam_tux_sphere_divisions(options.tux_sphere_divisions);
	setparam_draw_particles(options.draw_particles);
	setparam_track_marks(options.track_marks);

	write_config_file();
}

void update_view_slider()
{
	switch (view_mode)
	{
	case 0:
		widget_set_text(view_slider, "View Mode: Behind");
		break;
	case 1:
		widget_set_text(view_slider, "View Mode: Follow");
		break;
	case 2:
		widget_set_text(view_slider, "View Mode: Above");
		break;
	case 3:
		widget_set_text(view_slider, "View Mode: Tux's Eyes");
		break;
	}

	setparam_view_mode(view_mode);

	write_config_file();
}

void update_fps()
{
	if (display_fps)
	{
		widget_set_text(fps_btn, "Show FPS: Yes");
	}
	else
	{
		widget_set_text(fps_btn, "Show FPS: No");
	}

	setparam_display_fps(display_fps);

	write_config_file();
}

void music_volume_down(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	music_volume_ticks=GameMenu_resolve_bounds(music_volume_ticks-1, 0, 10, input_type);
	update_volume();
}
void music_volume_up(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	music_volume_ticks=GameMenu_resolve_bounds(music_volume_ticks+1, 0, 10, input_type);
	update_volume();
}

void sound_volume_down(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	sound_volume_ticks=GameMenu_resolve_bounds(sound_volume_ticks-1, 0, 10, input_type);
	update_volume();
}
void sound_volume_up(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	sound_volume_ticks=GameMenu_resolve_bounds(sound_volume_ticks+1, 0, 10, input_type);
	update_volume();
}

void back_click(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	set_game_mode( GAME_TYPE_SELECT );
	reset_gui();
}

void graphics_up(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	graphics_ticks=GameMenu_resolve_bounds(graphics_ticks+1, 0, NUM_GRAPHICS_OPTIONS-1, input_type);
	update_graphics();
}
void graphics_down(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	graphics_ticks=GameMenu_resolve_bounds(graphics_ticks-1, 0, NUM_GRAPHICS_OPTIONS-1, input_type);
	update_graphics();
}

void view_up(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	view_mode=GameMenu_resolve_bounds(view_mode+1, 1, NUM_VIEW_OPTIONS-1, input_type);
	update_view_slider();
}
void view_down(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	view_mode=GameMenu_resolve_bounds(view_mode-1, 1, NUM_VIEW_OPTIONS-1, input_type);
	update_view_slider();
}

void fps_click(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type)
{
	if (display_fps)
		display_fps=False;
	else
		display_fps=True;
	update_fps();
}

static void prefs_init(void) 
{
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( GameMenu_mouse_func );
    winsys_set_motion_func( GameMenu_motion_func );
    winsys_set_passive_motion_func( GameMenu_motion_func );
	winsys_set_joystick_func( GameMenu_joystick_func );
	winsys_set_joystick_button_func( GameMenu_joystick_button_func );

	winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_B, SDLK_ESCAPE);

	GameMenu_init();
	setup_gui();

	gui_add_widget(back_btn=create_button("Back", back_click), NULL);

	music_volume_slider=create_slider("", music_volume_down, music_volume_up);
	gui_add_widget(music_volume_slider, NULL);

	sound_volume_slider=create_slider("", sound_volume_down, sound_volume_up);
	gui_add_widget(sound_volume_slider, NULL);

	graphics_slider=create_slider("", graphics_down, graphics_up);
	gui_add_widget(graphics_slider, NULL);

	view_slider=create_slider("", view_down, view_up);
	gui_add_widget(view_slider, NULL);

	gui_add_widget(fps_btn=create_button("", fps_click), NULL);
	
	music_volume_ticks=getparam_music_volume();
	sound_volume_ticks=getparam_sound_volume();

	graphics_ticks=getparam_graphics_slider_tick();

	view_mode=getparam_view_mode();

	display_fps=getparam_display_fps();

	update_volume();
	update_graphics();
	update_fps();
	update_view_slider();

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

	GameMenu_draw();

    ui_draw_cursor();
    
    reshape( width, height );
    
    winsys_swap_buffers();
} 

START_KEYBOARD_CB( prefs_key_cb )
{
	if (release) return;

	switch (key)
	{
	case SDLK_ESCAPE:
	case SDLK_q:
	case SDLK_AC_BACK:
		GameMenu_simulate_click(back_btn);
		break;
	default:
	    GameMenu_keypress(key);
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


