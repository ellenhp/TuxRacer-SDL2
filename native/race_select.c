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
#include "race_select.h"
#include "ui_mgr.h"
#include "ui_theme.h"
#include "button.h"
#include "ssbutton.h"
#include "shaders.h"
#include "listbox.h"
#include "loop.h"
#include "render_util.h"
#include "audio.h"
#include "gl_util.h"
#include "keyboard.h"
#include "multiplayer.h"
#include "course_load.h"
#include "fonts.h"
#include "textures.h"
#include "course_mgr.h"
#include "textarea.h"
#include "save.h"
#include "game_logic_util.h"
#include "ui_snow.h"
#include "joystick.h"
#include "hud_training.h"
#include "gui_abstraction.h"
#include "gui_label.h"
#include "gui_button.h"
#include "platform.h"
#include "primitive_draw.h"

#ifdef TARGET_OS_IPHONE
#import "sharedGeneralFunctions.h"
#endif

#ifdef TARGET_OS_IPHONE
#define SCREEN_WIDTH 320
#define RACE_DESC_X 170
#define SCREEN_HEIGHT 480
#define BOX_WIDTH 300
#define TEXT_LINE_Y 26
#define RACE_DESC_Y 147
#define BUTTON_LINE_Y 151
#define MARGIN_WIDTH 10
#define MARGIN_HEIGHT 10
#else
#define SCREEN_WIDTH 640
#define RACE_DESC_X 312
#define SCREEN_HEIGHT 480
#define BOX_WIDTH 460
#define TEXT_LINE_Y 66
#define RACE_DESC_Y 107
#define BUTTON_LINE_Y 181
#define MARGIN_WIDTH 10
#define MARGIN_HEIGHT 42
#endif

static textarea_t *desc_ta = NULL;
static list_elem_t cur_elem = NULL;
static bool_t cup_complete = False;            /* has this cup been completed? */
static list_elem_t last_completed_race = NULL; /* last race that's been won */
static event_data_t *event_data = NULL;
static cup_data_t *cup_data = NULL;
static list_t race_list = NULL;
static player_data_t *plyr = NULL;

static widget_t *course_title_label = NULL;
static widget_t *prev_course_btn = NULL;
static widget_t *next_course_btn = NULL;
static widget_t *play_button = NULL;
static widget_t *back_button = NULL;

/* Forward declaration */
static void race_select_loop(scalar_t time_step);

const char *get_current_course_name()
{
    open_course_data_t *data;
    data = (open_course_data_t *)get_list_elem_data(cur_elem);
    return data->course;
}

/*---------------------------------------------------------------------------*/
/*! 
 Function used by listbox to convert list element to a string to display
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static char *get_name_from_race_data(list_elem_data_t elem)
{
    race_data_t *data;

    data = (race_data_t *)elem;
    return data->name;
}

/*---------------------------------------------------------------------------*/
/*! 
 Updates g_game.race to reflect current race data
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
void update_race_data(void)
{
    int i, course_index;

    if (g_game.practicing)
    {
        open_course_data_t *data;

        data = (open_course_data_t *)get_list_elem_data(cur_elem);
        g_game.race.course = data->course;
        g_game.race.name = data->name;
        g_game.race.description = data->description;
        g_game.race.conditions = data->conditions;
        for (i = 0; i < DIFFICULTY_NUM_LEVELS; i++)
        {
            g_game.race.herring_req[i] = 0;
            g_game.race.time_req[i] = 0;
            g_game.race.score_req[i] = 0;
        }

        g_game.race.time_req[0] = data->par_time;
    }
    else
    {
        race_data_t *data;
        data = (race_data_t *)get_list_elem_data(cur_elem);
        g_game.race = *data;
    }
}

/*---------------------------------------------------------------------------*/
/*! 
 Updates the race results based on the results of the race just completed
 Should be called before last_race_completed is updated.
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void update_race_results(void)
{
    char *event;
    char *cup;
    bool_t update_score = False;
    char *race_name;
    scalar_t time;
    int herring;
    int score;

    if (g_game.practicing)
    {
        open_course_data_t *data;
        data = (open_course_data_t *)get_list_elem_data(cur_elem);
        race_name = data->name;
    }
    else
    {
        race_data_t *data;
        data = (race_data_t *)get_list_elem_data(cur_elem);
        race_name = data->name;
    }

    event = g_game.current_event;
    cup = g_game.current_cup;

    if (!get_saved_race_results(plyr->name,
                                event,
                                cup,
                                race_name,
                                g_game.difficulty,
                                &time,
                                &herring,
                                &score))
    {
        update_score = True;
    }
    else if (!g_game.practicing && !cup_complete)
    {
        /* Scores are always overwritten if cup isn't complete */
        update_score = True;
    }
    else if (plyr->score > score)
    {
        update_score = True;
    }
    else
    {
        update_score = False;
    }

    if (update_score)
    {
        bool_t result;
        result =
            set_saved_race_results(plyr->name,
                                   event,
                                   cup,
                                   race_name,
                                   g_game.difficulty,
                                   g_game.time,
                                   plyr->herring,
                                   plyr->score);
        if (!result)
        {
            print_warning(IMPORTANT_WARNING,
                          "Couldn't save race results");
        }
    }
}

/*---------------------------------------------------------------------------*/
/*! 
 Call when a race has just been won
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
void update_for_won_race(void)
{
    race_data_t *race_data;

    check_assertion(g_game.practicing == False,
                    "Tried to update for won race in practice mode");

    race_data = (race_data_t *)get_list_elem_data(cur_elem);

    if (last_completed_race == NULL ||
        compare_race_positions(cup_data, last_completed_race,
                               cur_elem) > 0)
    {
        last_completed_race = cur_elem;

        if (cur_elem == get_list_tail(race_list))
        {
            cup_complete = True;

            if (!set_last_completed_cup(
                    plyr->name,
                    g_game.current_event,
                    g_game.difficulty,
                    g_game.current_cup))
            {
                print_warning(IMPORTANT_WARNING,
                              "Couldn't save cup completion");
            }
            else
            {
                print_debug(DEBUG_GAME_LOGIC,
                            "Cup %s completed",
                            g_game.current_cup);
            }
        }
    }
}

static void back()
{
    if (g_game.practicing)
    {
#ifdef SPEED_MODE
        set_game_mode(RACING_MODE_SELECT);
#else
        set_game_mode(GAME_TYPE_SELECT);
#endif
    }
    else
    {
        set_game_mode(GAME_TYPE_SELECT);
    }

    winsys_reset_js_bindings();

    ui_set_dirty();
}

static void start_race()
{
    race_select_loop(0);

    update_race_data();

    //Select the starting step
    if (!strcmp(g_game.race.name, "Basic tutorial"))
        init_starting_tutorial_step(0);
    if (!strcmp(g_game.race.name, "Jump tutorial"))
        init_starting_tutorial_step(10);

    winsys_reset_js_bindings();

    set_game_mode(LOADING);
}

/*---------------------------------------------------------------------------*/
/*! 
 Sets the widget positions and draws other on-screen goo 
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void draw_preview()
{
    GLuint texobj;

    {
        rect_t screen_rect;
        open_course_data_t *data;
        int w = getparam_x_resolution();
        int h = getparam_y_resolution();
        float colors[] = {1, 1, 1, 1};
        shader_set_color(colors);

        data = (open_course_data_t *)get_list_elem_data(cur_elem);
        if (!get_texture_binding(data->course, &texobj))
        {
            if (!get_texture_binding("no_preview", &texobj))
            {
                texobj = 0;
            }
        }

        glBindTexture(GL_TEXTURE_2D, texobj);

        draw_textured_quad(0.105 * w, 0.45 * h, 0.34 * w, 0.34 * h);
    }
}

void update_text()
{
    if (g_game.practicing)
    {
        open_course_data_t *data;
        data = (open_course_data_t *)get_list_elem_data(cur_elem);
        textarea_set_text(desc_ta, data->description);
        button_set_text(course_title_label, data->name);
        SDL_Log("update_text practice=%s", data->course);
        button_set_text(play_button, get_race_text());
    }
    else
    {
        race_data_t *data;

        data = (race_data_t *)get_list_elem_data(cur_elem);
        textarea_set_text(desc_ta, data->description);
        button_set_text(course_title_label, data->name);
        SDL_Log("update_text race=%s", data->course);
        button_set_text(play_button, get_race_text());
    }
}

void play_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t *widget)
{
    start_race();
}

void back_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t *widget)
{
    back();
}

void prev_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t *widget)
{
    if (get_prev_list_elem(race_list, cur_elem))
        cur_elem = get_prev_list_elem(race_list, cur_elem);
    update_text();
}

void next_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t *widget)
{
    if (get_next_list_elem(race_list, cur_elem))
        cur_elem = get_next_list_elem(race_list, cur_elem);
    update_text();
}

static void init_scoreboard()
{
    coord_t item_coord;
    int row;
    double tab_stops[] = {0.53, 0.61, 0.9};

    item_coord.x_coord_type = NORMALIZED_COORD;

    item_coord.y_coord_type = LINE_COORD;
    item_coord.y_just = CENTER_JUST;

    update_text();

    gui_balance_lines(0);
}

/*---------------------------------------------------------------------------*/
/*! 
 Mode initialization function
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
#define LEFT_ARROW "\x05"
#define RIGHT_ARROW "\x06"
static void race_select_init(void)
{
    point2d_t dummy_pos = {0, 0};
    coord_t button_coord;
    list_elem_t tmp;
    int i;

    winsys_set_display_func(main_loop);
    winsys_set_idle_func(main_loop);
    winsys_set_reshape_func(reshape);
    winsys_set_mouse_func(GameMenu_mouse_func);
    winsys_set_motion_func(GameMenu_motion_func);
    winsys_set_passive_motion_func(GameMenu_motion_func);
    winsys_set_joystick_func(NULL);
    winsys_set_joystick_button_func(NULL);

    winsys_reset_js_bindings();
    winsys_add_js_axis_bindings();
    winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_A, SDLK_RETURN);
    winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_B, SDLK_ESCAPE);

    winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDLK_LEFT);
    winsys_add_js_button_binding(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDLK_RIGHT);

    GameMenu_init();
    setup_gui();

    plyr = get_player_data(local_player());

    /* Setup the race list */
    if (g_game.practicing)
    {
        g_game.current_event = "__Practice_Event__";
        g_game.current_cup = "__Practice_Cup__";
#ifdef SPEED_MODE
        if (g_game.is_speed_only_mode)
            race_list = get_speed_courses_list();
        else
            race_list = get_score_courses_list();
#else
        race_list = get_score_courses_list();
#endif
        cup_data = NULL;
        last_completed_race = NULL;
        event_data = NULL;
    }

    /* Unless we're coming back from a race, initialize the race data to 
     defaults.
     */
    if (g_game.prev_mode != GAME_OVER)
    {
        /* Make sure we don't play previously loaded course */
        cup_complete = False;

        /* Initialize the race data */
        if (cur_elem == NULL)
        {
            cur_elem = get_list_head(race_list);
        }
        else
        {
            bool_t needs_changing = False;
            tmp = get_list_head(race_list);
            while (tmp != cur_elem && tmp != NULL)
            {
                tmp = get_next_list_elem(race_list, tmp);
            }
            if (!tmp)
            {
                cur_elem = get_list_head(race_list);
            }
        }

        if (g_game.practicing)
        {
            g_game.race.course = NULL;
            g_game.race.name = NULL;
            g_game.race.description = NULL;

            for (i = 0; i < DIFFICULTY_NUM_LEVELS; i++)
            {
                g_game.race.herring_req[i] = 0;
                g_game.race.time_req[i] = 0;
                g_game.race.score_req[i] = 0;
            }

            g_game.race.mirrored = False;
            g_game.race.conditions = RACE_CONDITIONS_SUNNY;
            g_game.race.windy = False;
            g_game.race.snowing = False;
        }
        else
        {
            /* Not practicing */

            race_data_t *data;
            data = (race_data_t *)get_list_elem_data(cur_elem);
            g_game.race = *data;

            if (is_cup_complete(event_data,
                                get_event_cup_by_name(
                                    event_data,
                                    g_game.current_cup)))
            {
                cup_complete = True;
                last_completed_race = get_list_tail(race_list);
            }
            else
            {
                cup_complete = False;
                last_completed_race = NULL;
            }
        }
    }
    else
    {
        /* Back from a race */
        if (!g_game.race_aborted)
        {
            update_race_results();
        }
    }

    /* 
     * Create text area 
     */
    desc_ta = textarea_create(make_point2d(
                                  0.1 * getparam_x_resolution(), 0.2 * getparam_y_resolution()),
                              0.4 * getparam_x_resolution(), 0.23 * getparam_y_resolution(), "race_description", "");

    textarea_set_visible(desc_ta, True);

    update_race_data();

    button_coord.x_coord_type = button_coord.y_coord_type = NORMALIZED_COORD;
    button_coord.x = 0.50;
    button_coord.y = 0.85;
    button_coord.x_just = CENTER_JUST;
    button_coord.y_just = CENTER_JUST;

    gui_add_widget(course_title_label = create_label(""), &button_coord);

    button_coord.x = 0.1;
    button_coord.x_just = LEFT_JUST;
    gui_add_widget(prev_course_btn = create_button(LEFT_ARROW, prev_cb), &button_coord);

    button_coord.x = 0.9;
    button_coord.x_just = RIGHT_JUST;
    gui_add_widget(next_course_btn = create_button(RIGHT_ARROW, next_cb), &button_coord);

    course_title_label->font_binding = "race_selection_title";
    prev_course_btn->font_binding = "race_selection_title";
    next_course_btn->font_binding = "race_selection_title";

    button_coord.x = 0.30;
    button_coord.y = 0.09;
    button_coord.x_just = CENTER_JUST;
    gui_add_widget(back_button = create_button(get_back_text(), back_cb), &button_coord);

    button_coord.x = 0.70;
    gui_add_widget(play_button = create_button(get_race_text(), play_cb), &button_coord);

    play_music("start_screen");
}

/*---------------------------------------------------------------------------*/
/*! 
 Mode loop function
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void race_select_loop(scalar_t time_step)
{
    use_hud_program();

    check_gl_error();

    update_audio();

    set_gl_options(GUI);

    clear_rendering_context();

    ui_setup_display();

    if (getparam_ui_snow())
    {
        update_ui_snow(time_step, False);
        draw_ui_snow();
    }

    ui_draw_menu_decorations(False);

    GameMenu_draw();

    draw_preview();

    winsys_update_joysticks();

    ui_draw();

    reshape(getparam_x_resolution(), getparam_y_resolution());

    winsys_swap_buffers();
}

/*---------------------------------------------------------------------------*/
/*! 
 Mode termination function
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void race_select_term(void)
{
    textarea_delete(desc_ta);
    desc_ta = NULL;
}

START_KEYBOARD_CB(race_select_key_cb)
{
    if (release)
    {
        return;
    }

    if (special)
    {
        switch (key)
        {
        case WSK_UP:
        case WSK_LEFT:
            if (get_prev_list_elem(race_list, cur_elem))
                cur_elem = get_prev_list_elem(race_list, cur_elem);
            update_text();
            break;
        case WSK_RIGHT:
        case WSK_DOWN:
            if (get_next_list_elem(race_list, cur_elem))
                cur_elem = get_next_list_elem(race_list, cur_elem);
            update_text();
            break;
        case SDLK_AC_BACK:
            back();
            break;
        }
    }
    else
    {
        key = (int)tolower((char)key);

        switch (key)
        {
        case 13: /* Enter */
            start_race();
            break;
        case 27: /* Esc */
            back();
            break;
        case 'c':
            //next_race_condition();
            break;
        case 'w':
            //toggle_wind();
            break;
        case 'm':
            //toggle_mirror();
            break;
        case 's':
            /* XXX snow disabled for now */
            break;
        }
    }

    ui_check_dirty();
}
END_KEYBOARD_CB

/*---------------------------------------------------------------------------*/
/*! 
 Mode registration function
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
void race_select_register()
{
    int status = 0;

    status |=
        add_keymap_entry(RACE_SELECT,
                         DEFAULT_CALLBACK, NULL, NULL, race_select_key_cb);

    check_assertion(status == 0,
                    "out of keymap entries");

    register_loop_funcs(RACE_SELECT,
                        race_select_init,
                        race_select_loop,
                        race_select_term);
}

/* EOF */
