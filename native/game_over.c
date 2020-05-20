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
#include "shaders.h"
#include "course_render.h"
#include "tux.h"
#include "tux_shadow.h"
#include "keyboard.h"
#include "loop.h"
#include "hud.h"
#include "hud_training.h"
#include "game_logic_util.h"
#include "fonts.h"
#include "ui_mgr.h"
#include "joystick.h"
#include "part_sys.h"
#ifdef TARGET_OS_IPHONE
#import "sharedGeneralFunctions.h"
#endif
#include "game_over.h"
#include "bonus.h"
#include "gui_abstraction.h"

static int next_mode = RACE_SELECT;

#define GAME_OVER_DISPLAY_TICKS 1000

static bool_t aborted = False;
static bool_t race_won = False;
static char *friendsRanking = NULL;
static char *countryRanking = NULL;
static char *worldRanking = NULL;
static scalar_t friendsPercent, countryPercent, worldPercent;
static int game_over_start_ticks = 0;

static void goto_next_mode()
{
    set_game_mode(next_mode);
    next_mode = RACE_SELECT;
}

static void mouse_cb(int button, int state, int finger_index, int x, int y)
{
#ifdef TARGET_OS_IPHONE
    if (g_game.practicing && !g_game.race_aborted && g_game.race.name != NULL && did_player_beat_best_results() && g_game.rankings_displayed == False)
    {
        //Notify that a new best result is for the moment unsaved
        //dirtyScores();
    }

    if (!did_player_beat_best_results() && !plyrWantsToDisplayRankingsAfterRace())
    {
        g_game.rankings_displayed = True;
    }

    if (!g_game.race_aborted && g_game.practicing && plyrWantsToSaveOrDisplayRankingsAfterRace() && g_game.rankings_displayed == False)
    {
        saveAndDisplayRankings();
    }
    else
    {
        //set landscape resolution
        setparam_x_resolution(320);
        setparam_y_resolution(480);

        //rotate screen
        turnScreenToPortrait();

        goto_next_mode();
        winsys_post_redisplay();
    }
#else
    if (SDL_GetTicks() - game_over_start_ticks > GAME_OVER_DISPLAY_TICKS)
    {
        goto_next_mode();
        winsys_post_redisplay();
    }
#endif
}

void game_over_set_next_mode(int mode)
{
    next_mode = mode;
}

/*---------------------------------------------------------------------------*/
/*! 
 Draws the text for the game over screen
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
void draw_game_over_text(void)
{
    const char *string;
    font_t *font;
    font_t *stat_label_font;
    coord_t coord;
    player_data_t *plyr = get_player_data(local_player());

    if (!g_game.race_aborted && g_game.practicing && g_game.needs_save_or_display_rankings)
    {
        string = Localize("World rankings", "");
    }
    else
        string = Localize("Race Over", "");

    coord.x = coord.y = 0.5;
    coord.x_coord_type = coord.y_coord_type = NORMALIZED_COORD;
    coord.x_just = coord.y_just = CENTER_JUST;
    GameMenu_draw_text(string, 0, coord, "race_over");
}

#ifdef TARGET_OS_IPHONE

//this function is called from scoreController.m in the function treatError
void displaySavedAndRankings(const char *msg, const char *friends, const char *country, const char *world, scalar_t friendsPercentage, scalar_t countryPercentage, scalar_t worldPercentage)
{
    free(friendsRanking);
    friendsRanking = strdup(friends);
    free(countryRanking);
    countryRanking = strdup(country);
    free(worldRanking);
    worldRanking = strdup(world);
    friendsPercent = friendsPercentage;
    countryPercent = countryPercentage;
    worldPercent = worldPercentage;
}

void saveAndDisplayRankings()
{
    //save score online if a best resul was established
    if (g_game.practicing && !g_game.race_aborted && g_game.race.name != NULL && did_player_beat_best_results())
    {
        int minutes;
        int seconds;
        int hundredths;
        player_data_t *plyr = get_player_data(local_player());
        get_time_components(g_game.time, &minutes, &seconds, &hundredths);
        //if the player choosed in his prefs not to save score online after ending a race but just to display rankings, the function below
        //will detect this case and redirect to the function displayRankingsAfterRace
    }
    //else display world rankings for this score
    else if (g_game.practicing && !g_game.race_aborted && g_game.race.name != NULL && !did_player_beat_best_results())
    {
        int minutes;
        int seconds;
        int hundredths;
        player_data_t *plyr = get_player_data(local_player());
        get_time_components(g_game.time, &minutes, &seconds, &hundredths);
    }
}
#endif

void game_over_js_func(int button)
{
    goto_next_mode();
    winsys_set_joystick_button_func(NULL);
    winsys_post_redisplay();
}

void game_over_init(void)
{
    winsys_set_display_func(main_loop);
    winsys_set_idle_func(main_loop);
    winsys_set_reshape_func(reshape);
    winsys_set_mouse_func(mouse_cb);
    winsys_set_motion_func(ui_event_motion_func);
    winsys_set_passive_motion_func(ui_event_motion_func);
    winsys_set_joystick_button_func(game_over_js_func);

    remove_all_bonuses();

    halt_sound("flying_sound");
    halt_sound("rock_sound");
    halt_sound("ice_sound");
    halt_sound("snow_sound");

    play_music("game_over");

    aborted = g_game.race_aborted;

    if (!aborted && g_game.practicing == True)
    {
        update_player_score(get_player_data(local_player()));
    }

    if ((!g_game.practicing && !aborted) || (!g_game.practicing && aborted && !game_abort_is_for_tutorial()))
    {
        race_won = was_current_race_won();
    }

    g_game.needs_save_or_display_rankings = False;
    g_game.rankings_displayed = False;

    game_over_start_ticks = SDL_GetTicks();
}

void game_over_loop(scalar_t time_step)
{
    player_data_t *plyr = get_player_data(local_player());
    int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();

    check_gl_error();

    new_frame_for_fps_calc();

    update_audio();

    clear_rendering_context();

    use_generic_program();

    update_player_pos(plyr, 0);
    update_view(plyr, 0);

    util_setup_projection(NEAR_CLIP_DIST, getparam_forward_clip_distance());

    set_course_clipping(True);
    set_course_eye_point(plyr->view.pos);

    use_terrain_program();

    render_course();

    use_generic_program();

    if (getparam_draw_particles())
    {
        draw_particles(plyr);
    }

    draw_tux();

    use_generic_program();

    draw_sky(plyr->view.pos);

    draw_trees();

    use_hud_program();

    set_gl_options(GUI);

    ui_setup_display();

    draw_game_over_text();

#ifndef TARGET_OS_IPHONE
    draw_hud(plyr);
#endif
    draw_hud_training(plyr);

    reshape(width, height);

    winsys_swap_buffers();
}

START_KEYBOARD_CB(game_over_cb)
{
    if (release)
        return;
#ifdef TARGET_OS_IPHONE

    if (g_game.practicing && !g_game.race_aborted && g_game.race.name != NULL && did_player_beat_best_results() && g_game.rankings_displayed == False)
    {
        //Notify that a new best result is for the moment unsaved
        //dirtyScores();
    }

    if (!did_player_beat_best_results() && !plyrWantsToDisplayRankingsAfterRace())
    {
        g_game.rankings_displayed == True;
    }

    if (!g_game.race_aborted && g_game.practicing && plyrWantsToSaveOrDisplayRankingsAfterRace() && g_game.rankings_displayed == False)
    {
        saveAndDisplayRankings();
    }
    else
    {
        //set landscape resolution
        setparam_x_resolution(320);
        setparam_y_resolution(480);

        //rotate screen
        turnScreenToPortrait();

        goto_next_mode();
        winsys_post_redisplay();
    }
#else
    goto_next_mode();
    winsys_post_redisplay();
#endif
}
END_KEYBOARD_CB

void game_over_register()
{
    int status = 0;

    status |= add_keymap_entry(GAME_OVER,
                               DEFAULT_CALLBACK, NULL, NULL, game_over_cb);

    check_assertion(status == 0, "out of keymap entries");

    register_loop_funcs(GAME_OVER, game_over_init, game_over_loop, NULL);
}
