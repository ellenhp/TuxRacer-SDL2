

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
#include "gui_slider.h"
#include "gui_button.h"
#include "gui_abstraction.h"
#include "joystick.h"

#define VOLUME_TOTAL_TICKS 10
#define VOLUME_SCALE		12

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
} graphics_options_t;

#define NUM_GRAPHICS_OPTIONS 3
graphics_options_t graphics_options[]={
	{5, 75, 0, False, False, False, 9, False, False},
	{10, 150, 20, True, False, False, 12, True, False},
	{20, 350, 70, True, True, True, 15, True, True},
};

widget_t* music_volume_slider=NULL;
widget_t* sound_volume_slider=NULL;

widget_t* graphics_slider=NULL;

#define NUM_VIEW_OPTIONS 4
widget_t* view_slider=NULL;

widget_t* fps_slider=NULL;

widget_t* prefs_select_button=NULL;
widget_t* prefs_back_button=NULL;

int graphics_ticks=0;

int view_mode=0;

bool_t display_fps;

void update_music_volume(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
	setparam_music_volume(widget->option * VOLUME_SCALE);
	write_config_file();
}

void update_sound_volume(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
	setparam_sound_volume(widget->option * VOLUME_SCALE);
	write_config_file();
}

void update_graphics(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
	graphics_options_t options;

	graphics_ticks=widget->option;
	options=graphics_options[graphics_ticks];
	
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

void update_view_mode(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
	setparam_view_mode(widget->option+1);
	write_config_file();
}

void update_fps(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
	if (widget->option)
		setparam_display_fps(True);
	else
		setparam_display_fps(False);

	write_config_file();
}

void prefs_select_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
    GameMenu_keypress(SDLK_RETURN);
}

void prefs_back_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget)
{
    set_game_mode( GAME_TYPE_SELECT );
}

static void prefs_init(void)
{
    coord_t button_coord;

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
    GameMenu_set_y_offset(-0.1);

	music_volume_slider=create_slider("Music Volume: ", 11, NULL, update_music_volume);
	slider_set_value(music_volume_slider, getparam_music_volume() / VOLUME_SCALE);
	gui_add_widget(music_volume_slider, NULL);

	sound_volume_slider=create_slider("Effects Volume: ", 11, NULL, update_sound_volume);
	slider_set_value(sound_volume_slider, getparam_sound_volume() / VOLUME_SCALE);
	gui_add_widget(sound_volume_slider, NULL);

	graphics_slider=create_slider("Graphics Setting: ", 3, "Low|Medium|High", update_graphics);
	slider_set_value(graphics_slider, getparam_graphics_slider_tick());
	gui_add_widget(graphics_slider, NULL);

	view_slider=create_slider("View Mode: ", 3, "Follow|Above|1st Person", update_view_mode);
	slider_set_value(view_slider, getparam_view_mode()-1);
	gui_add_widget(view_slider, NULL);

	fps_slider=create_slider("Show FPS: ", 2, "No|Yes", update_fps);
	slider_set_value(fps_slider, getparam_display_fps());
	gui_add_widget(fps_slider, NULL);
    
	display_fps=getparam_display_fps();

#ifdef __ANDROID__
    scoreloop_add_widgets();
#endif
    
    gui_balance_lines(0);
    
    if (is_on_ouya())
    {
        button_coord.x=0.30;
        button_coord.y=0.19;
        button_coord.x_just=button_coord.y_just=CENTER_JUST;
        button_coord.x_coord_type=button_coord.y_coord_type=NORMALIZED_COORD;
        gui_add_widget(prefs_back_button=create_label(get_back_text(), prefs_back_cb), &button_coord);
    }
    else
    {
        gui_add_widget(prefs_back_button=create_button(get_back_text(), prefs_back_cb), NULL);
    }
    
    if (is_on_ouya())
    {
        button_coord.x=0.70;
        button_coord.y=0.19;
        button_coord.x_just=button_coord.y_just=CENTER_JUST;
        button_coord.x_coord_type=button_coord.y_coord_type=NORMALIZED_COORD;
        gui_add_widget(prefs_select_button=create_label(get_select_text(), prefs_select_cb), &button_coord);
    }

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

	ui_draw_menu_decorations(True);
#ifdef __ANDROID__
    scoreloop_update_widgets();
#endif
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
        set_game_mode( GAME_TYPE_SELECT );
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


