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
#include "game_type_select.h"
#include "ui_mgr.h"
#include "ui_theme.h"
#include "button.h"
#include "loop.h"
#include "render_util.h"
#include "audio.h"
#include "gl_util.h"
#include "keyboard.h"
#include "multiplayer.h"
#include "ui_snow.h"
#include "joystick.h"
#include "textures.h"
#include "fonts.h"

#ifdef __APPLE__
    #include "sharedGeneralFunctions.h"
#endif

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#define LOGO_WIDTH 40
#define LOGO_HEIGHT 40
#define MARGIN_WIDTH 10
#define MARGIN_HEIGHT 10

static button_t *enter_speed_mode_btn = NULL;
static button_t *enter_fish_mode_btn = NULL;
static button_t *back_btn = NULL;
static char* commentaires[2];
static const char* logos_bindings[2];

static void release_commentaires() {
    free(commentaires[0]);
    free(commentaires[1]);
}

static void back_click_cb( button_t *button, void *userdata )
{
    check_assertion( userdata == NULL, "userdata is not null" );
    
    set_game_mode( GAME_TYPE_SELECT );
    
    ui_set_dirty();
}

void enter_speed_mode_click_cb( button_t* button, void *userdata )
{
    check_assertion( userdata == NULL, "userdata is not null" );
    
    release_commentaires();
    
    //g_game.is_speed_only_mode = True;
    set_game_mode( RACE_SELECT );
    
    ui_set_dirty();
}

void enter_fish_mode_click_cb( button_t* button, void *userdata )
{
    check_assertion( userdata == NULL, "userdata is not null" );
    
    release_commentaires();
    
    //g_game.is_speed_only_mode = False;
    set_game_mode( RACE_SELECT );
    
    ui_set_dirty();
}

static void set_widget_positions()
{
    button_t **button_list[] = {  &enter_speed_mode_btn, &enter_fish_mode_btn};
    font_t* font;
    int h = getparam_y_resolution();
    int box_height;
    int box_max_y;
    int top_y;
    int bottom_y;
    int num_buttons = sizeof( button_list ) / sizeof( button_list[0] );
    int i;
    int tot_button_height = 0;
    int button_sep =0;
    int cur_y_pos;
    
    box_height = 140;
    box_max_y = h - 50;
    
    bottom_y = 0.4*h - box_height/2+20;
    
    if ( bottom_y + box_height > box_max_y ) {
        bottom_y = box_max_y - box_height;
    }
    
    top_y = bottom_y + box_height;
    
    for (i=0; i<num_buttons; i++) {
        tot_button_height += button_get_height( *button_list[i] );
    }
    
    if ( num_buttons > 1 ) {
        button_sep = ( top_y - bottom_y - tot_button_height ) / 
	    ( num_buttons - 1 );
        button_sep = max( 0, button_sep );
    }
    
    cur_y_pos = top_y;
    
    /* for each button */
    for (i=0; i<num_buttons; i++) {
		int x_org, y_org;
        GLuint texobj;

        /* display_button */
        cur_y_pos -= button_get_height( *button_list[i] );
        button_set_position( 
                            *button_list[i],
                            make_point2d((SCREEN_WIDTH  - (LOGO_WIDTH + button_get_width( *button_list[i] )))/2.0+LOGO_WIDTH, cur_y_pos ) );
                            
        /* display comment under button */
        if ( !get_font_binding( "racing_mode_info", &font ) ) {
            print_warning( IMPORTANT_WARNING,
                          "Couldn't get font for binding racing_mode_info" );
        } else {
            int w, asc, desc;
            bind_font_texture( font );
            get_font_metrics( font, commentaires[i], &w, &asc, &desc );
            glPushMatrix();
            {
                glTranslatef( (SCREEN_WIDTH  - w)/2.0,cur_y_pos - MARGIN_HEIGHT , 0);
                draw_string( font, commentaires[i] );
            }
            glPopMatrix();
        }
        
        /* display logo at the left of the button */
        x_org = (SCREEN_WIDTH  - (LOGO_WIDTH + button_get_width( *button_list[i] )))/2.0;
        y_org = cur_y_pos;
        
        if ( !get_texture_binding( logos_bindings[i], &texobj ) ) {
            texobj = 0;
        }
        
        glBindTexture( GL_TEXTURE_2D, texobj );
        
        glBegin( GL_QUADS );
        {
            point2d_t tll, tur;
            point2d_t ll, ur;
            
            ll = make_point2d( x_org, y_org);
            ur = make_point2d( x_org + LOGO_WIDTH, y_org + LOGO_HEIGHT );
            tll = make_point2d( 0, 0 );
            tur = make_point2d(1, 1 );
            
            glTexCoord2f( tll.x, tll.y );
            glVertex2f( ll.x, ll.y );
            
            glTexCoord2f( tur.x, tll.y );
            glVertex2f( ur.x, ll.y );
            
            glTexCoord2f( tur.x, tur.y );
            glVertex2f( ur.x, ur.y );
            
            glTexCoord2f( tll.x, tur.y );
            glVertex2f( ll.x, ur.y );
        }
        glEnd();
        
        /* change cur_y_pos */
        cur_y_pos -= button_sep;
    }
    
    /* sets back button position */
    button_set_position( back_btn, make_point2d( MARGIN_WIDTH , MARGIN_HEIGHT ) );
}

static void racing_mode_select_init(void)
{
    point2d_t dummy_pos = {0, 0};
    
    winsys_set_display_func( main_loop );
    winsys_set_idle_func( main_loop );
    winsys_set_reshape_func( reshape );
    winsys_set_mouse_func( ui_event_mouse_func );
    winsys_set_motion_func( ui_event_motion_func );
    winsys_set_passive_motion_func( ui_event_motion_func );
    
    enter_fish_mode_btn = button_create( dummy_pos,
                                        300, 40,
                                        "button_label",
                                        Localize(" Classic mode","") );
    button_set_hilit_font_binding( enter_fish_mode_btn, "button_label_hilit" );
    button_set_visible( enter_fish_mode_btn, True );
    button_set_click_event_cb( enter_fish_mode_btn, enter_fish_mode_click_cb, NULL );
    
    enter_speed_mode_btn = button_create( dummy_pos,
                                         300, 40, 
                                         "button_label", 
                                         Localize("Time Trial","") );
    button_set_hilit_font_binding( enter_speed_mode_btn, "button_label_hilit" );
    button_set_visible( enter_speed_mode_btn, True );
    button_set_click_event_cb( enter_speed_mode_btn, enter_speed_mode_click_cb, NULL );
    
    back_btn = button_create( dummy_pos,
                             150, 40, 
                             "button_label", 
                             Localize("Back","") );
    button_set_hilit_font_binding( back_btn, "button_label_hilit" );
    button_set_visible( back_btn, True );
    button_set_click_event_cb( back_btn, back_click_cb, NULL );
    
    commentaires[0]=strdup(Localize("Just go fast!",""));
    commentaires[1]=strdup(Localize("Go fast and catch fishes...",""));
    
    logos_bindings[0]="time_icon";
    logos_bindings[1]="herring_icon";
    
    play_music( "start_screen" );
}

static void racing_mode_select_loop( scalar_t time_step )
{
    check_gl_error();
    
    update_audio();
    
    set_gl_options( GUI );
    
    clear_rendering_context();
    
    ui_setup_display();
    
    if (getparam_ui_snow()) {
        update_ui_snow( time_step, False );
        draw_ui_snow();
    }
    
    ui_draw_menu_decorations();
    
    set_widget_positions();
    
    ui_draw();
    
    reshape( getparam_x_resolution(), getparam_y_resolution() );
    
    winsys_swap_buffers();
}

static void racing_mode_select_term(void)
{
    button_delete( enter_speed_mode_btn );
    enter_speed_mode_btn = NULL;
    
    button_delete( enter_fish_mode_btn );
    enter_fish_mode_btn = NULL;
    
    button_delete( back_btn );
    back_btn = NULL;
}

START_KEYBOARD_CB( racing_mode_select_cb )
{
    if (release) return;
    
    winsys_post_redisplay();
}
END_KEYBOARD_CB

void racing_mode_select_register()
{
    int status = 0;
    
    /*status |=
	add_keymap_entry( RACING_MODE_SELECT,
                     DEFAULT_CALLBACK, NULL, NULL, racing_mode_select_cb );
    register_loop_funcs( RACING_MODE_SELECT, 
                        racing_mode_select_init,
                        racing_mode_select_loop,
                        racing_mode_select_term );*/
    
}


/* EOF */
