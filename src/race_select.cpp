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
#ifdef TARGET_OS_IPHONE
	#import "sharedGeneralFunctions.h"
#endif

extern "C"
{

#ifdef TARGET_OS_IPHONE
	#define SCREEN_WIDTH	320
	#define RACE_DESC_X		170
	#define SCREEN_HEIGHT	480
	#define BOX_WIDTH		300
	#define TEXT_LINE_Y		26
	#define RACE_DESC_Y		147
	#define BUTTON_LINE_Y	151
	#define MARGIN_WIDTH	10
	#define MARGIN_HEIGHT	10
#else
	#define SCREEN_WIDTH	640
	#define RACE_DESC_X		312
	#define SCREEN_HEIGHT	480
	#define BOX_WIDTH		460
	#define TEXT_LINE_Y		66
	#define RACE_DESC_Y		107
	#define BUTTON_LINE_Y	181
	#define MARGIN_WIDTH	10
	#define MARGIN_HEIGHT	42
#endif

static textarea_t *desc_ta = NULL;
static listbox_t *race_listbox = NULL;
static button_t  *back_btn = NULL;
static button_t  *start_btn = NULL;
static ssbutton_t *conditions_ssbtn = NULL;
static ssbutton_t *snow_ssbtn = NULL;
static ssbutton_t *wind_ssbtn = NULL;
static ssbutton_t *mirror_ssbtn = NULL;
static list_elem_t cur_elem = NULL;
static bool_t cup_complete = False; /* has this cup been completed? */
static list_elem_t last_completed_race = NULL; /* last race that's been won */
static event_data_t *event_data = NULL;
static cup_data_t *cup_data = NULL;
static list_t race_list = NULL;
static player_data_t *plyr = NULL;

/* Forward declaration */
static void race_select_loop( scalar_t time_step );

/*---------------------------------------------------------------------------*/
/*! 
 Function used by listbox to convert list element to a string to display
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static char* get_name_from_open_course_data( list_elem_data_t elem )
{
    open_course_data_t *data;
    
    data = (open_course_data_t*) elem;
    return data->name;
}


/*---------------------------------------------------------------------------*/
/*! 
 Function used by listbox to convert list element to a string to display
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static char* get_name_from_race_data( list_elem_data_t elem )
{
    race_data_t *data;
    
    data = (race_data_t*) elem;
    return data->name;
}


#if 0
/*---------------------------------------------------------------------------*/
/*! 
  Returns true iff the current race is completed
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static bool_t is_current_race_completed( void )
{
    check_assertion( cur_elem != NULL, "current race is null" );
    
    if ( last_completed_race == NULL ) {
        return False;
    }
    
    if ( compare_race_positions( cup_data, cur_elem,
                                last_completed_race ) >= 0 )
    {
        return True;
    }
    
    return False;
}


/*---------------------------------------------------------------------------*/
/*! 
  Returns true iff the current race is the first incomplete race
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static bool_t is_current_race_first_incomplete( void )
{
    check_assertion( cur_elem != NULL, "current race is null" );
    
    if ( last_completed_race == NULL ) {
        if ( cur_elem == get_list_head( race_list ) ) {
            return True;
        } else {
            return False;
        }
    }
    
    if ( compare_race_positions( cup_data, last_completed_race,
                                cur_elem ) == 1 )
    {
        return True;
    }
    
    return False;
}

#endif

/*---------------------------------------------------------------------------*/
/*! 
 Updates g_game.race to reflect current race data
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
void update_race_data( void )
{
    int i;
    
    if ( g_game.practicing ) {
        open_course_data_t *data;
        
        data = (open_course_data_t*) get_list_elem_data( cur_elem );
        g_game.race.course = data->course;
        g_game.race.name = data->name;
        g_game.race.description = data->description;
        g_game.race.conditions = data->conditions;
        ssbutton_set_state( conditions_ssbtn, (int) data->conditions );
        for (i=0; i<DIFFICULTY_NUM_LEVELS; i++) {
            g_game.race.herring_req[i] = 0;
            g_game.race.time_req[i] = 0;
            g_game.race.score_req[i] = 0;
        }
        
        g_game.race.time_req[0] = data->par_time;
        
        g_game.race.mirrored = (bool_t) ssbutton_get_state( mirror_ssbtn );
		//g_game.race.conditions = (race_conditions_t) ssbutton_get_state(conditions_ssbtn );
        g_game.race.windy = (bool_t) ssbutton_get_state( wind_ssbtn );
        g_game.race.snowing = (bool_t) ssbutton_get_state( snow_ssbtn );
    } else {
        race_data_t *data;
        data = (race_data_t*) get_list_elem_data( cur_elem );
        g_game.race = *data;
        
        if ( cup_complete && 
            mirror_ssbtn != NULL &&
            conditions_ssbtn != NULL &&
            wind_ssbtn != NULL &&
            snow_ssbtn != NULL )
        {
            /* If the cup is complete, allowed to customize settings */
            g_game.race.mirrored = (bool_t) ssbutton_get_state( mirror_ssbtn );
            g_game.race.conditions = (race_conditions_t) ssbutton_get_state(
                                                                            conditions_ssbtn );
            g_game.race.windy = (bool_t) ssbutton_get_state( wind_ssbtn );
            g_game.race.snowing = (bool_t) ssbutton_get_state( snow_ssbtn );
        }
    }
}


/*---------------------------------------------------------------------------*/
/*! 
 Updates the enabled states of the buttons
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
void update_button_enabled_states( void )
{
    if ( start_btn == NULL ) {
        return;
    }
    button_set_enabled( start_btn, True );
}


/*---------------------------------------------------------------------------*/
/*! 
 Updates the race results based on the results of the race just completed
 Should be called before last_race_completed is updated.
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void update_race_results( void )
{
    char *event;
    char *cup;
    bool_t update_score = False;
    char *race_name;
    scalar_t time;
    int herring;
    int score;
    
    if ( g_game.practicing ) {
        open_course_data_t *data;
        data = (open_course_data_t*)get_list_elem_data( cur_elem );
        race_name = data->name;
    } else {
        race_data_t *data;
        data = (race_data_t*)get_list_elem_data( cur_elem );
        race_name = data->name;
    }
    
    event = g_game.current_event;
    cup = g_game.current_cup;
    
    if ( !get_saved_race_results( plyr->name,
                                 event,
                                 cup,
                                 race_name,
                                 g_game.difficulty,
                                 &time,
                                 &herring,
                                 &score ) )
    {
        update_score = True;
    } else if ( !g_game.practicing && !cup_complete ) {
        /* Scores are always overwritten if cup isn't complete */
        update_score = True;
    } else if ( plyr->score > score ) {
        update_score = True;
    } else {
        update_score = False;
    }
    
    if ( update_score ) {
        bool_t result;
        result = 
	    set_saved_race_results( plyr->name,
                               event,
                               cup,
                               race_name,
                               g_game.difficulty,
                               g_game.time,
                               plyr->herring,
                               plyr->score ); 
        if ( !result ) {
            print_warning( IMPORTANT_WARNING,
                          "Couldn't save race results" );
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
void update_for_won_race( void )
{
    race_data_t *race_data;
    
    check_assertion( g_game.practicing == False,
                    "Tried to update for won race in practice mode" );
    
    race_data = (race_data_t*)get_list_elem_data( cur_elem );
    
    if ( last_completed_race == NULL ||
        compare_race_positions( cup_data, last_completed_race, 
                               cur_elem ) > 0 )
    {
        last_completed_race = cur_elem;
		
        if ( cur_elem == get_list_tail( race_list ) ) {
            cup_complete = True;
            
            if ( !set_last_completed_cup( 
                                         plyr->name,
                                         g_game.current_event,
                                         g_game.difficulty,
                                         g_game.current_cup ) )
            {
                print_warning( IMPORTANT_WARNING,
                              "Couldn't save cup completion" );
            } else {
                print_debug( DEBUG_GAME_LOGIC, 
                            "Cup %s completed", 
                            g_game.current_cup );
            }
        }
    }
	
    update_button_enabled_states();
}



/*---------------------------------------------------------------------------*/
/*! 
 Callback called when race listbox item changes
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void race_listbox_item_change_cb( listbox_t *listbox, void *userdata )
{
    check_assertion( userdata == NULL, "userdata is not null" );
    
    cur_elem = listbox_get_current_item( listbox );
    
    if ( g_game.practicing ) {
        open_course_data_t *data;
        
        data = (open_course_data_t*) get_list_elem_data( cur_elem );
        textarea_set_text( desc_ta, data->description );
        
        ui_set_dirty();
    } else {
        race_data_t *data;
        
        data = (race_data_t*) get_list_elem_data( cur_elem );
        textarea_set_text( desc_ta, data->description );
        
        if ( cup_complete && 
            conditions_ssbtn &&
            wind_ssbtn &&
            snow_ssbtn &&
            mirror_ssbtn ) 
        {
            ssbutton_set_state( conditions_ssbtn,
                               (int) data->conditions );
            ssbutton_set_state( wind_ssbtn,
                               (int) data->windy );
            ssbutton_set_state( snow_ssbtn,
                               (int) data->snowing );
            ssbutton_set_state( mirror_ssbtn,
                               (int) data->mirrored );
        }
        
        update_button_enabled_states();
        
        ui_set_dirty();
    } 
    
    update_race_data();
}


/*---------------------------------------------------------------------------*/
/*! 
 Callback called when back button is clicked
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void back_click_cb( button_t *button, void *userdata )
{
    check_assertion( userdata == NULL, "userdata is not null" );
    
    if ( g_game.practicing ) {
#ifdef SPEED_MODE
        set_game_mode( RACING_MODE_SELECT );
#else
        set_game_mode( GAME_TYPE_SELECT );
#endif
    } else {
        set_game_mode( GAME_TYPE_SELECT );
    }
    
	winsys_reset_js_bindings();

    ui_set_dirty();
}


/*---------------------------------------------------------------------------*/
/*! 
 Callback called when start button is clicked
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void start_click_cb( button_t *button, void *userdata )
{
    check_assertion( userdata == NULL, "userdata is not null" );
    
    button_set_highlight( start_btn, True );
    race_select_loop( 0 );
    
    update_race_data();
    
#ifdef TARGET_OS_IPHONE
    //set landscape resolution
    setparam_x_resolution(480);
    setparam_y_resolution(320);
    
    //rotate screen
    turnScreenToLandscape();
#endif
    
    //Select the starting step
    if (!strcmp(g_game.race.name,"Basic tutorial")) init_starting_tutorial_step(0);
    if (!strcmp(g_game.race.name,"Jump tutorial")) init_starting_tutorial_step(10);
    
	winsys_reset_js_bindings();

    set_game_mode( LOADING );

}


/*---------------------------------------------------------------------------*/
/*! 
 Draws a status message on the screen
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
void draw_status_msg( int x_org, int y_org, int box_width, int box_height )
{
    const char *msg;
    scalar_t time;
    int herring;
    int score;
    font_t *label_font;
    font_t *font;
    char buff[BUFF_LEN];
    bool_t draw_stats = True;
    
    if ( g_game.practicing ) {
        open_course_data_t *data;
        
        data = (open_course_data_t*)get_list_elem_data( cur_elem );
        
        msg = Localize("Best result:","");
        
        if ( !get_saved_race_results( plyr->name,
                                     g_game.current_event,
                                     g_game.current_cup,
                                     data->name,
                                     g_game.difficulty,
                                     &time,
                                     &herring,
                                     &score ) )
        {
            /* Don't display anything if no score saved */
            return;
        }
        
        
#ifdef TARGET_OS_IPHONE
#define yTranslation -10
#else
#define yTranslation 0
#endif
        
        if ( !get_font_binding( "race_requirements", &font ) ||
            !get_font_binding( "race_requirements_label", &label_font ) ) 
        {
            print_warning( IMPORTANT_WARNING,
                          "Couldn't get fonts for race requirements" );
        } else {
            glPushMatrix();
            {
                glTranslatef( x_org + 0,
                             y_org + 200+yTranslation,
                             0 );
                
                bind_font_texture( label_font );
                draw_string( label_font, msg );
            }
            glPopMatrix();
            
            if ( draw_stats ) {
                glPushMatrix();
                {
                    int minutes;
                    int seconds;
                    int hundredths;
                    
                    get_time_components( time, &minutes, &seconds, &hundredths );
                    
                    glTranslatef( x_org + 0,
                                 y_org + 184+yTranslation,
                                 0 );
                    
                    
                    bind_font_texture( label_font );
                    draw_string( label_font, Localize("Time: ","") );
                    
                    sprintf( buff, "%02d:%02d.%02d",
                            minutes, seconds, hundredths );
                    bind_font_texture( font );
                    draw_string( font, buff );

                    if (game_has_herring()) {
                        bind_font_texture( label_font );
                        draw_string( label_font, Localize("    Fish: ","") );
                        
                        sprintf( buff, "%03d", herring ); 
                        bind_font_texture( font );
                        draw_string( font, buff );
                        
                        bind_font_texture( label_font );
                        draw_string( label_font, Localize("     Score: ","") );
                        
                        sprintf( buff, "%06d", score );
                        bind_font_texture( font );
                        draw_string( font, buff );
                    }
                }
                glPopMatrix();
            }
        }
    }
}

/*---------------------------------------------------------------------------*/
/*! 
 Sets the widget positions and draws other on-screen goo 
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void set_widget_positions_and_draw_decorations()
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    int box_width, box_height, box_max_y;
    int x_org, y_org;
    char *current_course;
    GLuint texobj;
    
    /* set the dimensions of the box in which all widgets should fit */
    box_width = BOX_WIDTH;
    box_height = 310;
    box_max_y = h - 128;

#ifdef TARGET_OS_IPHONE
    x_org = 10;
    y_org = h/2 - box_height/2;

    y_org = h/2 - box_height/2;
    
    if ( y_org + box_height > box_max_y ) {
        y_org = box_max_y - box_height;
    }
    
    button_set_position( 
                        back_btn,
                        make_point2d(MARGIN_WIDTH,
                                     MARGIN_HEIGHT ) );
    
    button_set_position(
                        start_btn,
                        make_point2d(w - button_get_width( start_btn ) - MARGIN_WIDTH,
                                     MARGIN_HEIGHT ) );
    
    listbox_set_position(
                         race_listbox,
                         make_point2d( x_org,
                                      y_org + 235 ) );
    textarea_set_position( 
                          desc_ta,
                          make_point2d( x_org + w - textarea_get_width(desc_ta) - 10,
                                       y_org + TEXT_LINE_Y));
#else
    x_org = w/2 - box_width/2;
    y_org = h/2 - box_height/2;

    button_set_position( 
	back_btn,
	make_point2d( x_org + 131 - button_get_width( back_btn )/2.0,
		      42 ) );

    button_set_position(
	start_btn,
	make_point2d( x_org + 343 - button_get_width( start_btn )/2.0,
		      42 ) );

    listbox_set_position(
	race_listbox,
	make_point2d( x_org,
		      y_org + 221 ) );

    textarea_set_position( 
	desc_ta,
	make_point2d( x_org,
		      y_org + 66 ) );
#endif

    if ( g_game.practicing || 
        ( cup_complete &&
         conditions_ssbtn &&
         wind_ssbtn &&
         snow_ssbtn &&
         mirror_ssbtn ) ) 
    {
        ssbutton_set_position(
                              conditions_ssbtn,
                              make_point2d( x_org + box_width - 4*36 + 4,
                                           y_org + BUTTON_LINE_Y ) );
        
        ssbutton_set_position(
                              wind_ssbtn,
                              make_point2d( x_org + box_width - 3*36 + 4 ,
                                           y_org + BUTTON_LINE_Y ) );
        
        ssbutton_set_position(
                              snow_ssbtn,
                              make_point2d( x_org + box_width - 2*36 + 4,
                                           y_org + BUTTON_LINE_Y ) );
        
        ssbutton_set_position(
                              mirror_ssbtn,
                              make_point2d( x_org + box_width - 1*36 + 4,
                                           y_org + BUTTON_LINE_Y ) );
        
    } 
    
    /* Draw text indicating race requirements (if race not completed), 
     or results in race if race completed. */
    draw_status_msg( x_org, y_org, box_width, box_height );
    
    /* Draw preview */
    if ( g_game.practicing ) {
        list_elem_t elem;
        open_course_data_t *data;
        
        elem = listbox_get_current_item( race_listbox );
        data = (open_course_data_t*) get_list_elem_data( elem );
        current_course = data->course;
    } else {
        list_elem_t elem;
        race_data_t *data;
        
        elem = listbox_get_current_item( race_listbox );
        data = (race_data_t*) get_list_elem_data( elem );
        current_course = data->course;
    }
    
    glDisable( GL_TEXTURE_2D );
    
    glColor4f( 0.0, 0.0, 0.0, 0.3 );
    
    GLfloat vertices[]={
		x_org+box_width-140, y_org+66, 0,
		x_org+box_width, y_org+66, 0,
		x_org+box_width, y_org+66+107, 0,
		x_org+box_width-140, y_org+66+107, 0};

	GLubyte indices[] = {0, 1, 2, 2, 3, 0};

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
		
	glDisableClientState(GL_VERTEX_ARRAY);


    
    glColor4f( 1.0, 1.0, 1.0, 1.0 );
    glEnable( GL_TEXTURE_2D );
    
    if ( !get_texture_binding( current_course, &texobj ) ) {
        if ( !get_texture_binding( "no_preview", &texobj ) ) {
            texobj = 0;
        }
    }
    
    glBindTexture( GL_TEXTURE_2D, texobj );

	GLfloat texcoords2[]={
		0, 0,
		1, 0,
		1, 1,
		0, 1};

	GLfloat vertices2[]={
		x_org+box_width-136, y_org+70, 0,
		x_org+box_width-4, y_org+70, 0,
		x_org+box_width-4, y_org+70+99, 0,
		x_org+box_width-136, y_org+70+99, 0};
    
	GLubyte indices2[] = {0, 1, 2, 2, 3, 0};

    glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glTexCoordPointer(2, GL_FLOAT, 0, texcoords2);
	glVertexPointer(3, GL_FLOAT, 0, vertices2);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices2);
		
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


}


/*---------------------------------------------------------------------------*/
/*! 
 Mode initialization function
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void race_select_init(void)
{
    listbox_list_elem_to_string_fptr_t conv_func = NULL;
    point2d_t dummy_pos = {0, 0};
    int i;
    
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( ui_event_mouse_func );
    winsys_set_motion_func( ui_event_motion_func );
    winsys_set_passive_motion_func( ui_event_motion_func );
    
	winsys_add_js_axis_bindings();
	winsys_add_js_button_binding(getparam_joystick_continue_button(), SDLK_RETURN);
	winsys_add_js_button_binding(getparam_joystick_brake_button(), SDLK_ESCAPE);

    plyr = get_player_data( local_player() );
    
    /* Setup the race list */
    if ( g_game.practicing ) {
        g_game.current_event = "__Practice_Event__";
        g_game.current_cup = "__Practice_Cup__";
#ifdef SPEED_MODE
        if(g_game.is_speed_only_mode)
            race_list = get_speed_courses_list();
        else
            race_list = get_score_courses_list();
#else
        race_list = get_score_courses_list();
#endif
        conv_func = get_name_from_open_course_data;
        cup_data = NULL;
        last_completed_race = NULL;
        event_data = NULL;
    } else {
        event_data = (event_data_t*) get_list_elem_data( 
                                                        get_event_by_name( g_game.current_event ) );
        check_assertion( event_data != NULL,
                        "Couldn't find current event." );
        cup_data = (cup_data_t*) get_list_elem_data(
                                                    get_event_cup_by_name( event_data, g_game.current_cup ) );
        check_assertion( cup_data != NULL,
                        "Couldn't find current cup." );
        race_list = get_cup_race_list( cup_data );
        conv_func = get_name_from_race_data;
    }
    
    /* Unless we're coming back from a race, initialize the race data to 
     defaults.
     */
    if ( g_game.prev_mode != GAME_OVER ) {
        /* Make sure we don't play previously loaded course */
        cup_complete = False;
        
        /* Initialize the race data */
        cur_elem = get_list_head( race_list );
        
        if ( g_game.practicing ) {
            g_game.race.course = NULL;
            g_game.race.name = NULL;
            g_game.race.description = NULL;
            
            for (i=0; i<DIFFICULTY_NUM_LEVELS; i++) {
                g_game.race.herring_req[i] = 0;
                g_game.race.time_req[i] = 0;
                g_game.race.score_req[i] = 0;
            }
            
            g_game.race.mirrored = False;
            g_game.race.conditions = RACE_CONDITIONS_SUNNY;
            g_game.race.windy = False;
            g_game.race.snowing = False;
        } else {
            /* Not practicing */
            
            race_data_t *data;
            data = (race_data_t*) get_list_elem_data( cur_elem );
            g_game.race = *data;
            
            if ( is_cup_complete( event_data, 
                                 get_event_cup_by_name( 
                                                       event_data, 
                                                       g_game.current_cup ) ) )
            {
                cup_complete = True;
                last_completed_race = get_list_tail( race_list );
            } else {
                cup_complete = False;
                last_completed_race = NULL;
            }
        }
    } else {
        /* Back from a race */
        if ( !g_game.race_aborted ) {
            update_race_results();
        }
        
    }
    
    back_btn = button_create( dummy_pos,
                             150, 40, 
                             "button_label", 
                             Localize("Back","") );
    button_set_hilit_font_binding( back_btn, "button_label_hilit" );
    button_set_visible( back_btn, True );
    button_set_click_event_cb( back_btn, back_click_cb, NULL );
    
    start_btn = button_create( dummy_pos,
                              150, 40,
                              "button_label",
                              Localize("Race!","") );
    button_set_hilit_font_binding( start_btn, "button_label_hilit" );
    button_set_disabled_font_binding( start_btn, "button_label_disabled" );
    button_set_visible( start_btn, True );
    button_set_click_event_cb( start_btn, start_click_cb, NULL );
    
#ifdef TARGET_OS_IPHONE
    race_listbox = listbox_create( dummy_pos,
                                  300, 44,
                                  "course_name_label",
                                  race_list,
                                  conv_func );
    
#else
    race_listbox = listbox_create( dummy_pos,
                                  460, 44,
                                  "listbox_item",
                                  race_list,
                                  conv_func );
    
#endif
    
    
    listbox_set_current_item( race_listbox, cur_elem );
    
    listbox_set_item_change_event_cb( race_listbox, 
                                     race_listbox_item_change_cb, 
                                     NULL );
    
    listbox_set_visible( race_listbox, True );
    
    /* 
     * Create text area 
     */
    desc_ta = textarea_create( dummy_pos,
                              RACE_DESC_X, RACE_DESC_Y,
                              "race_description",
                              "" );

    if ( g_game.practicing ) {
        open_course_data_t *data;
        data = (open_course_data_t*) get_list_elem_data( cur_elem );
        textarea_set_text( desc_ta, data->description );
    } else {
        race_data_t *data;
        data = (race_data_t*) get_list_elem_data( cur_elem );
        textarea_set_text( desc_ta, data->description );
    }
    
    textarea_set_visible( desc_ta, True );
    
    
    /* 
     * Create state buttons - only if practicing or if cup_complete
     */
    
    if ( g_game.practicing || cup_complete ) {
        float border = 2.0;

        /* mirror */
        mirror_ssbtn = ssbutton_create( dummy_pos,
                                       32, 32,
                                       2 );
        ssbutton_set_state_image( mirror_ssbtn, 
                                 0, 
                                 "mirror_button",
                                 make_point2d( 0.0/64.0, 32.0/64.0 ),
                                 make_point2d( 32.0/64.0, 64.0/64.0 ),
                                 white );
        
        ssbutton_set_state_image( mirror_ssbtn, 
                                 1, 
                                 "mirror_button",
                                 make_point2d( 32.0/64.0, 32.0/64.0 ),
                                 make_point2d( 64.0/64.0, 64.0/64.0 ),
                                 white );
        
        ssbutton_set_state( mirror_ssbtn, (int)g_game.race.mirrored );
#ifdef TARGET_OS_IPHONE
        ssbutton_set_visible( mirror_ssbtn, False );
#else
        ssbutton_set_visible( mirror_ssbtn, True );
#endif
        
        /* conditions */
        conditions_ssbtn = ssbutton_create( dummy_pos,
                                           32, 32,
                                           4 );
        
        ssbutton_set_state_image( conditions_ssbtn, 
                                 0, 
                                 "conditions_button",
                                 make_point2d( (0.0 + border)/64.0, (32.0 + border)/64.0 ),
                                 make_point2d( (32.0 - border)/64.0, (64.0 - border)/64.0 ),
                                 white );
        
        ssbutton_set_state_image( conditions_ssbtn, 
                                 1, 
                                 "conditions_button",
                                 make_point2d( (32.0 + border)/64.0, (0.0 + border)/64.0 ),
                                 make_point2d( (64.0 - border)/64.0, (32.0 - border)/64.0 ),
                                 white );
        
        ssbutton_set_state_image( conditions_ssbtn, 
                                 2, 
                                 "conditions_button",
                                 make_point2d( (32.0 + border)/64.0, (32.0 + border)/64.0 ),
                                 make_point2d( (64.0 - border)/64.0, (64.0 - border)/64.0 ),
                                 white );
        
        ssbutton_set_state_image( conditions_ssbtn, 
                                 3, 
                                 "conditions_button",
                                 make_point2d( (0.0 + border)/64.0, (0.0 + border)/64.0 ),
                                 make_point2d( (32.0 - border)/64.0, (32.0 - border)/64.0 ),
                                 white );
        
        ssbutton_set_state( conditions_ssbtn, (int)g_game.race.conditions );
        ssbutton_set_visible( conditions_ssbtn, True );
        
#ifdef TARGET_OS_IPHONE
        ssbutton_set_visible( conditions_ssbtn, False );
#else
        ssbutton_set_visible( conditions_ssbtn, True );
#endif
        
        /* wind */
        wind_ssbtn = ssbutton_create( dummy_pos,
                                     32, 32,
                                     2 );
        ssbutton_set_state_image( wind_ssbtn, 
                                 0, 
                                 "wind_button",
                                 make_point2d( 0.0/64.0, 32.0/64.0 ),
                                 make_point2d( 32.0/64.0, 64.0/64.0 ),
                                 white );
        
        ssbutton_set_state_image( wind_ssbtn, 
                                 1, 
                                 "wind_button",
                                 make_point2d( 32.0/64.0, 32.0/64.0 ),
                                 make_point2d( 64.0/64.0, 64.0/64.0 ),
                                 white );
        
        ssbutton_set_state( wind_ssbtn, (int)g_game.race.windy );
#ifdef TARGET_OS_IPHONE
        ssbutton_set_visible( wind_ssbtn, False );
#else
        ssbutton_set_visible( wind_ssbtn, True );
#endif
        
        /* snow */
        snow_ssbtn = ssbutton_create( dummy_pos,
                                     32, 32,
                                     2 );
        ssbutton_set_state_image( snow_ssbtn, 
                                 0, 
                                 "snow_button",
                                 make_point2d( 0.0/64.0, 32.0/64.0 ),
                                 make_point2d( 32.0/64.0, 64.0/64.0 ),
                                 white );
        
        ssbutton_set_state_image( snow_ssbtn, 
                                 1, 
                                 "snow_button",
                                 make_point2d( 32.0/64.0, 32.0/64.0 ),
                                 make_point2d( 64.0/64.0, 64.0/64.0 ),
                                 white );
        
        ssbutton_set_state( snow_ssbtn, (int)g_game.race.snowing );
#ifdef TARGET_OS_IPHONE
        ssbutton_set_visible( snow_ssbtn, False );
#else
        ssbutton_set_visible( snow_ssbtn, True );
#endif
        /* XXX snow button doesn't do anything, so disable for now */
        ssbutton_set_enabled( snow_ssbtn, False );
        
        /* Can't change conditions if in cup mode */
        if ( !g_game.practicing ) {
            ssbutton_set_enabled( conditions_ssbtn, False );
            ssbutton_set_enabled( wind_ssbtn, False );
            ssbutton_set_enabled( snow_ssbtn, False );
            ssbutton_set_enabled( mirror_ssbtn, False );
        }
        
    } else {
        conditions_ssbtn = NULL;
        wind_ssbtn = NULL;
        snow_ssbtn = NULL;
        mirror_ssbtn = NULL;
    }
    
    update_race_data();
    update_button_enabled_states();
    
    play_music( "start_screen" );
}


/*---------------------------------------------------------------------------*/
/*! 
 Mode loop function
 \author  jfpatry
 \date    Created:  2000-09-24
 \date    Modified: 2000-09-24
 */
static void race_select_loop( scalar_t time_step )
{
    check_gl_error();
    
    update_audio();
    
    set_gl_options( GUI );
    
    clear_rendering_context();
    
    ui_setup_display();
    
    if (getparam_ui_snow()) {
        update_ui_snow( time_step, 
                       (bool_t) ( wind_ssbtn != NULL && 
                                 ssbutton_get_state( wind_ssbtn ) ) );
        draw_ui_snow();
    }
    
    ui_draw_menu_decorations();
    
    set_widget_positions_and_draw_decorations();
    
	winsys_update_joysticks();

    ui_draw();
    
    reshape( getparam_x_resolution(), getparam_y_resolution() );
    
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
    if ( back_btn ) {
        button_delete( back_btn );
    }
    back_btn = NULL;
    
    if ( start_btn ) {
        button_delete( start_btn );
    }
    start_btn = NULL;
    
    if ( race_listbox ) {
        listbox_delete( race_listbox );
    }
    race_listbox = NULL;
    
    if ( conditions_ssbtn ) {
        ssbutton_delete( conditions_ssbtn );
    }
    conditions_ssbtn = NULL;
    
    if ( snow_ssbtn ) {
        ssbutton_delete( snow_ssbtn );
    }
    snow_ssbtn = NULL;
    
    if ( wind_ssbtn ) {
        ssbutton_delete( wind_ssbtn );
    }
    wind_ssbtn = NULL;
    
    if ( mirror_ssbtn ) {
        ssbutton_delete( mirror_ssbtn );
    }
    mirror_ssbtn = NULL;
    
    textarea_delete( desc_ta );
    desc_ta = NULL;
}


/*---------------------------------------------------------------------------*/
/*! 
 Advances to the next race condition
 \author  jfpatry
 \date    Created:  2000-09-30
 \date    Modified: 2000-09-30
 */
void next_race_condition( void )
{
    if ( conditions_ssbtn ) {
        ssbutton_simulate_mouse_click( conditions_ssbtn );
    }
}


/*---------------------------------------------------------------------------*/
/*! 
 Toggles the mirrored state of the course
 \author  jfpatry
 \date    Created:  2000-09-30
 \date    Modified: 2000-09-30
 */
void toggle_mirror( void )
{
    if ( mirror_ssbtn ) {
        ssbutton_simulate_mouse_click( mirror_ssbtn );
    }
}


/*---------------------------------------------------------------------------*/
/*! 
 Toggles the windy state of the course
 \author  jfpatry
 \date    Created:  2000-09-30
 \date    Modified: 2000-09-30
 */
void toggle_wind( void )
{
    if ( wind_ssbtn ) {
        ssbutton_simulate_mouse_click( conditions_ssbtn );
    }
}


START_KEYBOARD_CB( race_select_key_cb )
{
    if ( release ) {
        return;
    }
    
    if ( special ) {
        switch (key) {
            case WSK_UP:
            case WSK_LEFT:
                if ( race_listbox ) {
                    listbox_goto_prev_item( race_listbox );
                }
                break;
            case WSK_RIGHT:
            case WSK_DOWN:
                if ( race_listbox ) {
                    listbox_goto_next_item( race_listbox );
                }
                break;
        }
    } else {
        key = (int) tolower( (char) key );
        
        switch (key) {
            case 13: /* Enter */
                if ( start_btn ) {
                    button_simulate_mouse_click( start_btn );
                    ui_set_dirty();
                }
                break;
            case 27: /* Esc */
                if ( back_btn ) {
                    button_simulate_mouse_click( back_btn );
                    ui_set_dirty();
                }
                break;
            case 'c': 
                next_race_condition();
                break;
            case 'w': 
                toggle_wind();
                break;
            case 'm':
                toggle_mirror();
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
    add_keymap_entry( RACE_SELECT,
                     DEFAULT_CALLBACK, NULL, NULL, race_select_key_cb );
    
    check_assertion( status == 0,
                    "out of keymap entries" );
    
    register_loop_funcs( RACE_SELECT, 
                        race_select_init,
                        race_select_loop,
                        race_select_term );
}

}

/* EOF */
