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
#include "fog.h"
#include "viewfrustum.h"
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

#define NEXT_MODE RACE_SELECT
#define GAME_OVER_DISPLAY_TICKS 1000

static bool_t aborted = False;
static bool_t race_won = False;
static char* friendsRanking = NULL;
static char* countryRanking = NULL;
static char* worldRanking = NULL;
static scalar_t friendsPercent,countryPercent,worldPercent;
static int game_over_start_ticks=0;

static void mouse_cb( int button, int state, int finger_index, int x, int y )
{
#ifdef TARGET_OS_IPHONE
    if (g_game.practicing && !g_game.race_aborted && g_game.race.name!=NULL && did_player_beat_best_results()  && g_game.rankings_displayed==False) {
        //Notify that a new best result is for the moment unsaved
        //dirtyScores();
    }
    
    if (!did_player_beat_best_results() && !plyrWantsToDisplayRankingsAfterRace()) {
        g_game.rankings_displayed=True;
    }
    
    if (!g_game.race_aborted && g_game.practicing && plyrWantsToSaveOrDisplayRankingsAfterRace() && g_game.rankings_displayed==False) {
        saveAndDisplayRankings();
    }
    else 
    {
        //set landscape resolution
        setparam_x_resolution(320);
        setparam_y_resolution(480);
        
        //rotate screen
        turnScreenToPortrait();
        
        set_game_mode( NEXT_MODE );
        winsys_post_redisplay();
    }
#else
	if (SDL_GetTicks()-game_over_start_ticks>GAME_OVER_DISPLAY_TICKS)
	{
	    set_game_mode( NEXT_MODE );
		winsys_post_redisplay();
	}
#endif
}


/*---------------------------------------------------------------------------*/
/*! 
 Draws the text for the game over screen
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
void draw_game_over_text( void )
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    int x_org, y_org;
    int box_width, box_height;
    const char *string;
    int string_w, asc, desc;
    char buff[BUFF_LEN];
    font_t *font;
    font_t *stat_label_font;
    player_data_t *plyr = get_player_data( local_player() );
    
    box_width = 200;
    box_height = 250;
    
    x_org = w/2.0 - box_width/2.0;
    y_org = h/2.0 - box_height/2.0;
    
    if ( !get_font_binding( "race_over", &font ) ) {
        print_warning( IMPORTANT_WARNING,
                      "Couldn't get font for binding race_over" );
    } else {
    
        if ( !g_game.race_aborted && g_game.practicing && g_game.needs_save_or_display_rankings) {
            string = Localize("World rankings", "");
        } else string = Localize("Race Over", "");

        get_font_metrics( font, string, &string_w, &asc, &desc );
        
        glPushMatrix();
        {
            glTranslatef( x_org + box_width/2.0 - string_w/2.0,
                         y_org + box_height - asc, 
                         0 );
            bind_font_texture( font );
            draw_string( font, string );
        }
        glPopMatrix();
    }
    
    /* If race was aborted, don't print stats, and if doesnt need to print rankings, dont print them */

    if ( !g_game.race_aborted && !g_game.needs_save_or_display_rankings && g_game.practicing) 

        {
            if ( !get_font_binding( "race_stats_label", &stat_label_font ) ||
                !get_font_binding( "race_stats", &font ) )
            {
                print_warning( IMPORTANT_WARNING,
                              "Couldn't get fonts for race stats" );
            } else {
                int asc2;
                int desc2;
                get_font_metrics( font, "", &string_w, &asc, &desc );
                get_font_metrics( stat_label_font, "", &string_w, &asc2, &desc2 );
                
                if ( asc < asc2 ) {
                    asc = asc2;
                }
                if ( desc < desc2 ) {
                    desc = desc2;
                }
                
                glPushMatrix();
                {
                    int minutes;
                    int seconds;
                    int hundredths;
                    
                    glTranslatef( x_org,
                                 y_org + 150,
                                 0 );
                    
                    bind_font_texture( stat_label_font );
                    
                    if (!strcmp(get_calculation_mode(),"jump") )
                    {
                        
                        draw_string( stat_label_font, Localize("Flying time: ","src/game_over.c"));
                        g_game.time = plyr->control.fly_total_time;
                    }
                    //default mode
                    else {
                        draw_string( stat_label_font, Localize("Time: ","src/game_over.c"));
                    }
                    get_time_components( g_game.time, &minutes, &seconds, &hundredths );
                    sprintf( buff, "%02d:%02d:%02d", minutes, seconds, hundredths );
                    
                    bind_font_texture( font );
                    draw_string( font, buff );
                }
                glPopMatrix();
                
                glPushMatrix();
                {
                    glTranslatef( x_org,
                                 y_org + 150 - (asc + desc),
                                 0 );
                    
                    bind_font_texture( stat_label_font );
#ifdef SPEED_MODE
                    if (!g_game.is_speed_only_mode)
#endif
                        draw_string( stat_label_font, Localize("Fish: ","") );
                    
                    sprintf( buff, "%3d", plyr->herring );
                    
                    bind_font_texture( font );
#ifdef _SPEED_MODE
                    if (!g_game.is_speed_only_mode)
#endif
                        draw_string( font, buff );
                }
                glPopMatrix();
                
                glPushMatrix();
                {
                    glTranslatef( x_org,
                                 y_org + 150 - 2*(asc + desc),
                                 0 );
                    
                    bind_font_texture( stat_label_font );
#ifdef SPEED_MODE
                    if (!g_game.is_speed_only_mode)
#endif
                        draw_string( stat_label_font, "Score: " );
                    
                    sprintf( buff, "%6d", plyr->score );
                    
                    bind_font_texture( font );
                    
#ifdef SPEED_MODE
                    if (!g_game.is_speed_only_mode)
#endif
                        draw_string( font, buff );
                }
                glPopMatrix();
            }
        } 
#ifdef TARGET_OS_IPHONE
    //display rankings if needed
        else if ( !g_game.race_aborted && g_game.practicing && g_game.needs_save_or_display_rankings) 
        {
            if ( !get_font_binding( "race_stats_label", &stat_label_font ) ||
                !get_font_binding( "race_stats", &font ) )
            {
                print_warning( IMPORTANT_WARNING,
                              "Couldn't get fonts for race stats" );
            } else {
                int asc2;
                int desc2;
                get_font_metrics( font, "", &string_w, &asc, &desc );
                get_font_metrics( stat_label_font, "", &string_w, &asc2, &desc2 );
                
                if ( asc < asc2 ) {
                    asc = asc2;
                }
                if ( desc < desc2 ) {
                    desc = desc2;
                }
                
                glPushMatrix();
                {
                    glTranslatef( x_org - 50,
                                 y_org + 150,
                                 0 );
                    
                    bind_font_texture( stat_label_font );
                    draw_string( stat_label_font, Localize("Friends: ","") );
                    if (strcmp(friendsRanking, "Empty friends list.")==0) {
                        free(friendsRanking);
                        friendsRanking = strdup(Localize("No friends",""));
                    }
                    
                    if (friendsPercent>=0)
                        sprintf( buff, "%s (%.1f%%)", friendsRanking , friendsPercent);
                    else
                        sprintf( buff, "%s", friendsRanking);
                    
                    bind_font_texture( font );
                    draw_string( font, buff );
                }
                glPopMatrix();
                
                glPushMatrix();
                {
                    glTranslatef( x_org -50 ,
                                 y_org + 150 - (asc + desc),
                                 0 );
                    
                    bind_font_texture( stat_label_font );
                    draw_string( stat_label_font, Localize("Country: ","") );
                    
                    sprintf( buff, "%s (%.1f%%)", countryRanking , countryPercent );
                    
                    bind_font_texture( font );
                    draw_string( font, buff );
                }
                glPopMatrix();
                
                glPushMatrix();
                {
                    glTranslatef( x_org - 50,
                                 y_org + 150 - 2*(asc + desc),
                                 0 );
                    
                    bind_font_texture( stat_label_font );
                    draw_string( stat_label_font, Localize("World: ","") );
                    
                    sprintf( buff, "%s (%.1f%%)", worldRanking , worldPercent);
                    
                    bind_font_texture( font );
                    draw_string( font, buff );
                }
                glPopMatrix();
            }
        }
#endif
    
    if ( g_game.race_aborted && !g_game.race_time_over) {
        string = Localize("Race aborted.","");
    } else if ( g_game.race_aborted && g_game.race_time_over) {
        string = Localize("Time is up.","");
    } else if ( ( g_game.practicing || is_current_cup_complete() ) &&
               did_player_beat_best_results() ) 
    {
        
        if ( !g_game.race_aborted && g_game.practicing && g_game.needs_save_or_display_rankings) {
            string = "";
        } else string = Localize("You beat your best score!","");
        
    } else {
        string = "";
    } 
    
    if ( !get_font_binding( "race_result_msg", &font ) ) {
        print_warning( IMPORTANT_WARNING, 
                      "Couldn't get font for binding race_result_msg" );
    } else {
        get_font_metrics( font, string, &string_w, &asc, &desc );
        glPushMatrix();
        {
            glTranslatef( x_org + box_width/2. - string_w/2.,
                         y_org + desc +20.,
                         0 );
            bind_font_texture( font );
            draw_string( font, string );
        }
        glPopMatrix();
    }
#ifdef TARGET_OS_IPHONE
    //Draws "touch screen to contin"ue
    string = Localize("Touch screen to continue","");
    
    if ( !get_font_binding( "fps", &font ) ) {
        print_warning( IMPORTANT_WARNING, 
                      "Couldn't get font for binding race_result_msg" );
    } else {
        get_font_metrics( font, string, &string_w, &asc, &desc );
        glPushMatrix();
        {
            glTranslatef( x_org + box_width/2. - string_w/2.,
                         y_org + desc-20.,
                         0 );
            bind_font_texture( font );
            draw_string( font, string );
        }
        glPopMatrix();
    }
#endif   
    
}

#ifdef TARGET_OS_IPHONE

//this function is called from scoreController.m in the function treatError
void displaySavedAndRankings(const char* msg, const char* friends, const char* country, const char* world, scalar_t friendsPercentage, scalar_t countryPercentage , scalar_t worldPercentage) {
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

void saveAndDisplayRankings() { 
    //save score online if a best resul was established
    if (g_game.practicing && !g_game.race_aborted && g_game.race.name!=NULL && did_player_beat_best_results()) {
        int minutes;
        int seconds;
        int hundredths;
        player_data_t *plyr = get_player_data( local_player() );
        get_time_components( g_game.time, &minutes, &seconds, &hundredths);
        //if the player choosed in his prefs not to save score online after ending a race but just to display rankings, the function below
        //will detect this case and redirect to the function displayRankingsAfterRace
    } 
    //else display world rankings for this score
    else if (g_game.practicing && !g_game.race_aborted && g_game.race.name!=NULL && !did_player_beat_best_results()) {
        int minutes;
        int seconds;
        int hundredths;
        player_data_t *plyr = get_player_data( local_player() );
        get_time_components( g_game.time, &minutes, &seconds, &hundredths);
    }
}
#endif

void game_over_js_func(int button)
{
    set_game_mode( NEXT_MODE );
	winsys_set_joystick_button_func(NULL);
    winsys_post_redisplay();
}

void game_over_init(void) 
{
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( mouse_cb );
    winsys_set_motion_func( ui_event_motion_func );
    winsys_set_passive_motion_func( ui_event_motion_func );
	winsys_set_joystick_button_func( game_over_js_func );
    
    remove_all_bonuses();
    
    halt_sound( "flying_sound" );
    halt_sound( "rock_sound" );
    halt_sound( "ice_sound" );
    halt_sound( "snow_sound" );
    
    play_music( "game_over" );
    
    aborted = g_game.race_aborted;
    
    if ( !aborted ) {
        update_player_score( get_player_data( local_player() ) );
    }
    
    if ( (!g_game.practicing &&!aborted) || (!g_game.practicing && aborted && !game_abort_is_for_tutorial())) {
        race_won = was_current_race_won();
        init_starting_tutorial_step(-100);
    }
    
    g_game.needs_save_or_display_rankings=False;
    g_game.rankings_displayed=False;

	game_over_start_ticks=SDL_GetTicks();
}

void game_over_loop( scalar_t time_step )
{
    player_data_t *plyr = get_player_data( local_player() );
    int width, height;
    width = getparam_x_resolution();
    height = getparam_y_resolution();
    
    check_gl_error();
    
    new_frame_for_fps_calc();
    
    update_audio();
    
    clear_rendering_context();
    
    setup_fog();
    
    update_player_pos( plyr, 0 );
    update_view( plyr, 0 );
    
    setup_view_frustum( plyr, NEAR_CLIP_DIST, 
                       getparam_forward_clip_distance() );
    
    draw_sky(plyr->view.pos);
    
    draw_fog_plane();
    
    set_course_clipping( True );
    set_course_eye_point( plyr->view.pos );
    setup_course_lighting();
    render_course();
    draw_trees();
    
    if ( getparam_draw_particles() ) {
        draw_particles( plyr );
    }
    
    draw_tux();
    
    set_gl_options( GUI );
    
    ui_setup_display();
    
    draw_game_over_text();
    
#ifndef TARGET_OS_IPHONE
    draw_hud( plyr );
#endif
    draw_hud_training(plyr);
    reshape( width, height );
    
    winsys_swap_buffers();
} 

START_KEYBOARD_CB( game_over_cb )
{
    if ( release ) return;
#ifdef TARGET_OS_IPHONE
    
    if (g_game.practicing && !g_game.race_aborted && g_game.race.name!=NULL && did_player_beat_best_results()  && g_game.rankings_displayed==False) {
        //Notify that a new best result is for the moment unsaved
        //dirtyScores();
    }
    
    if (!did_player_beat_best_results() && !plyrWantsToDisplayRankingsAfterRace()) {
        g_game.rankings_displayed==True;
    }
    
    if (!g_game.race_aborted && g_game.practicing && plyrWantsToSaveOrDisplayRankingsAfterRace() && g_game.rankings_displayed==False) {
        saveAndDisplayRankings();
    }
    else 
    {
        //set landscape resolution
        setparam_x_resolution(320);
        setparam_y_resolution(480);
        
        //rotate screen
        turnScreenToPortrait();
        
        set_game_mode( NEXT_MODE );
        winsys_post_redisplay();
    }
#else
    set_game_mode( NEXT_MODE );
    winsys_post_redisplay();
#endif
    
}
END_KEYBOARD_CB

void game_over_register()
{
    int status = 0;
    
    status |= add_keymap_entry( GAME_OVER, 
                               DEFAULT_CALLBACK, NULL, NULL, game_over_cb );
    
    check_assertion( status == 0, "out of keymap entries" );
    
    register_loop_funcs( GAME_OVER, game_over_init, game_over_loop, NULL );
}


