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
#include "fonts.h"
#include "gl_util.h"
#include "textures.h"
#include "fps.h"
#include "phys_sim.h"
#include "multiplayer.h"
#include "ui_mgr.h"
#include "game_logic_util.h"
#include "course_load.h"
#include "fonts.h"
#include "loop.h"
#include "racing.h"
#include "paused.h"
#ifdef TARGET_OS_IPHONE
    #include "sharedGeneralFunctions.h"
#endif

static int step = -1;
static  uint32_t first_time_true = 0;
static bool_t is_condition_verified = False;
static bool_t training_abort = False;
static bool_t pause_for_long_tutorial_explanation = False;
static bool_t resume_from_tutorial_explanation = False;


static void print_instruction(const char* string, int line) {

    char* binding = "instructions";
    font_t *font;
    int w, asc, desc;
    
    if ( !get_font_binding( binding, &font ) ) {
        print_warning( IMPORTANT_WARNING,
                      "Couldn't get font for binding %s", binding );
        return;
    }
    
    bind_font_texture( font );
    
    get_font_metrics( font, (char*)string, &w, &asc, &desc );
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0,1.0,1.0,0.4);
    
	{
    GLfloat vertices[]={
		0.0, (float)(200-(line-2)*(asc+desc)) -5.0, 0,
		0.0, (float)(200-(line-1)*(asc+desc)) -5.0, 0,
		480.0, (float)(200-(line-1)*(asc+desc)) -5.0, 0,
		480.0, (float)(200-(line-2)*(asc+desc)) -5.0, 0};

	GLubyte indices[] = {0, 1, 2, 2, 3, 0};

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
		
	glDisableClientState(GL_VERTEX_ARRAY);

    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    {
        glTranslatef( 240.0-(float)w/2.0,
                     200-(line-1)*(asc+desc),
                     0 );
        draw_string( font, (char*)string );
    }
    glPopMatrix();
	}
}

static void drawRedCircle(GLint x, GLint y) {
    int x_org = x;
    int y_org = y;
    GLuint texobj;
    point2d_t tll, tur;
    point2d_t ll, ur;
    if ( !get_texture_binding( "red_circle", &texobj ) ) {
        texobj = 0;
    }
    glColor4f(1.0,0.0,0.0,0.5);
    
    glBindTexture( GL_TEXTURE_2D, texobj );

        
    ll = make_point2d( x_org, y_org);
    ur = make_point2d( x_org + 110, y_org + 110 );
    tll = make_point2d( 0, 0 );
    tur = make_point2d(1, 1 );
    
	{
 	GLfloat texcoords[]={
		tll.x, tll.y,
		tll.x, tur.y,
		tur.x, tur.y,
		tur.x, tll.y};

	GLfloat vertices[]={
		ll.x, ll.y, 0,
		ll.x, ur.y, 0,
		ur.x, ur.y, 0,
		ur.x, ll.y, 0};

	GLubyte indices[] = {0, 1, 2, 2, 3, 0};

    glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
		
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	}
}  

/* verifie qu'une condition a bien été vérifiée pendant au moins "sec" secondes */
static bool_t check_condition_for_time(bool_t condition,int sec) {
    uint32_t now;
    if (!condition) {
        first_time_true = get_clock_time();
        is_condition_verified=False;
        return False;
    }
    else {
        if (is_condition_verified==False) {
            first_time_true = get_clock_time();
        }
        is_condition_verified=True;
        now = get_clock_time();
        if ((now-first_time_true) >= sec) {
            is_condition_verified=False;
            return True;
        }
        else return False;
    }
    return False;
}

static void training_pause_for_tutorial_explanation(void)
{
    if(!pause_for_long_tutorial_explanation)
    {
        pause_for_long_tutorial_explanation = True;
        resume_from_tutorial_explanation = False;
        set_game_mode(PAUSED);
    }
}

static bool_t training_is_resumed(void)
{
    if(resume_from_tutorial_explanation)
    {
        resume_from_tutorial_explanation = False;
        pause_for_long_tutorial_explanation = False;
        return True;
    }
    return False;
}

bool_t game_abort_is_for_tutorial(void)
{
    return training_abort;
}

bool_t pause_is_for_long_tutorial_explanation(void)
{
    return pause_for_long_tutorial_explanation;
}

void training_resume_from_tutorial_explanation(void)
{
    resume_from_tutorial_explanation = True;
}



static void draw_instructions(player_data_t *plyr)
{
    switch (step) {
        case 0:
            print_instruction(Localize("Welcome to the basic tutorial.", ""),1);
            print_instruction(Localize("You will learn here how to turn, to", ""),2);
            print_instruction(Localize("accelerate, to brake to pause and to abort.", ""),3);
            //N'est affiché que endant l'intro
            if(g_game.time>0) step++;
            break;
        case 1:
            print_instruction(Localize("Try to make tux turn right.", ""),1);
            print_instruction(Localize("(Inclinate your iPhone.)", ""),2);
            if (plyr->control.turn_fact>0.5) step = 2;
            break;
        case 2:
            print_instruction(Localize("Try to make tux turn left.", ""),1);
            print_instruction(Localize("(Inclinate your iPhone.)", ""),2);
            if (plyr->control.turn_fact<-0.4) step = 3;
            break;
        case 3:
            print_instruction(Localize("Push the red area to make tux paddling.", ""),1);
            drawRedCircle(370.0, 0.0);
            if (check_condition_for_time(plyr->control.is_accelerating,2)) step = 4;
            break;
        case 4:
            training_pause_for_tutorial_explanation();
            print_instruction(Localize("While the speed indicator is green,", ""),1);
            print_instruction(Localize("paddling increases the speed of Tux.", ""),2);
            print_instruction(Localize("But when it becomes yellow, it has the ", ""),3);
            print_instruction(Localize("opposite effect. Then you must release", ""),4);
            print_instruction(Localize("it to continue going faster", ""),5);
            if(training_is_resumed()) step++;
            break;
        case 5:
            print_instruction(Localize("Push the red area to brake.", ""),1);
            print_instruction(Localize("(Braking is useful when going fast to", ""),2);
            print_instruction(Localize("turn harder).", ""),3);
            drawRedCircle(10, 0.0);
            if (check_condition_for_time(plyr->control.is_braking,2)) step = 6;
            break;
        case 6:
            print_instruction(Localize("Now let's learn how to Pause and to Abort.", ""),1);
            if(check_condition_for_time(True,2)) step = 7;
            break;
        case 7:
            print_instruction(Localize("Double tap in the middle of the screen", ""),1);
            print_instruction(Localize("to Pause.", ""),2);
            if(g_game.race_paused==True) step = 8;
            break;
        case 8:
            print_instruction(Localize("Cool! Now come back to game.", ""),0);
            print_instruction(Localize("Tap anywhere on the screen.", ""),1);
            if(g_game.race_paused==False) step = 9;
            break;
        case 9:
            print_instruction(Localize("Drag your finger from one area", ""),2);
            print_instruction(Localize("to the other to abort.", ""),3);
            drawRedCircle(10, 0.0);
            drawRedCircle(370.0, 200);
            if(g_game.race_aborted==True) step = -1;
            training_abort=True;
            break;
        case -1:
            print_instruction(Localize("Congratulation, you finished this tutorial.", ""),1);
            print_instruction(Localize("You can now try the next one, to learn", ""),2);
            print_instruction(Localize("some more advanced things and get better !", ""),3);
            set_game_mode( GAME_OVER );
            break;
            					/* Fin du premier Tutorial */
                                
                                /* Début du second Tutorial */
        case 10:
            print_instruction(Localize("Welcome to the Jump tutorial.", ""),1);
            print_instruction(Localize("You will learn here to jump, to fly,", ""),2);
            print_instruction(Localize("and to make tricks.", ""),3);
            //N'est affiché que endant l'intro
            if(g_game.time>0) step=11;
            break;
        case 11:
             print_instruction(Localize("Let's learn how to jump !", ""),1);
             if (check_condition_for_time( True,1)) step = 12;
            break;
        case 12:
            print_instruction(Localize("Push the red area to accumulate enough ", ""),2);
            print_instruction(Localize("energy to jump.", ""),3);
            drawRedCircle(370.0, 200);
            if (plyr->control.jump_charging) step = 13;
            break;
        case 13:
            print_instruction(Localize("You can see the energy gauge filling.", ""),1);
            print_instruction(Localize("When you release the finger, all the energy", ""),2);
            print_instruction(Localize("accumulated is released and used to jump.", ""),3);
            if (check_condition_for_time( True,6)) step = 14;
            break;
        case 14:
            print_instruction(Localize("Now try to do a jump.", ""),1);
            if (check_condition_for_time( True,1)) step = 15;
            break;
        case 15:
            print_instruction(Localize("Try to do a jump.", ""),-3);
            if (plyr->control.jumping) step = 16;
            break;
        case 16:
            print_instruction(Localize("Ok, now try to do a longer jump.", ""),1);
            print_instruction(Localize("(At least 1 second in the air.)", ""),2);
            print_instruction(Localize("Go fast and jump on a bump.", ""),3);
            if (check_condition_for_time( True,5)) step = 17;
            break;
        case 17:
            print_instruction(Localize("Try to do a longer jump (>1sec) on a bump.", ""),-3);
            if (check_condition_for_time(plyr->control.is_flying,1)) step = 18;
            break;
        case 18:
            print_instruction(Localize("Great !.", ""),1);
            if (check_condition_for_time( True,1)) step = 19;
            break;
        case 19:
            if (check_condition_for_time( True,1)) step = 20;
            break;
        case 20:
            training_pause_for_tutorial_explanation();
            print_instruction(Localize("Well, while you are in the air, there", ""),1);
            print_instruction(Localize("are two things that you can do : doing", ""),2);
            print_instruction(Localize("some funny tricks shaking the iPhone,", ""),3);
            print_instruction(Localize("and flapping the wings (accelerate button)", ""),4);
            print_instruction(Localize("to fly longer and get faster.", ""),5);
            if(training_is_resumed()) step++;
            break;
        case 21:
            training_pause_for_tutorial_explanation();
            print_instruction(Localize("First of all, find a bump, go fast", ""),1);
            print_instruction(Localize("on it, jump high and try to make a trick.", ""),2);
            print_instruction(Localize("You can vary the tricks inclinating the iPhone", ""),3);
            print_instruction(Localize("or pushing either brake or accelerating button", ""),4);
            print_instruction(Localize("before shaking your iPhone.", ""),5);
            if(training_is_resumed()) step++;
            break;
        case 22:
            print_instruction(Localize("Try to jump and make a trick.", ""),-3);
            if (plyr->tricks>0) step = 23;
            break;
        case 23:
            print_instruction(Localize("Great !.", ""),1);
            if (check_condition_for_time( True,2)) step = 24;
            break;
        case 24:
            training_pause_for_tutorial_explanation();
            print_instruction(Localize("Now, to finish, try to make a big jump ", ""),1);
            print_instruction(Localize("(at least 1 seconds in the air) on a big bump", ""),2);
            print_instruction(Localize("flapping your wings (pushing accelerate button).", ""),3);
            if(training_is_resumed()) {
                step++;
                step = 25;
                if ((plyr->pos.z)<(-200)) {
                    point_t p = make_point(48.0,-105.8,-200.0);
                    racing_init_for_tutorial(p);
                }
            }
            break;
        case 25:
            print_instruction(Localize("Try to do a long jump flying (>1sec).", ""),-3);
            if (check_condition_for_time( (bool_t)(plyr->control.is_flying && plyr->control.is_accelerating),1)) step = -2;
            break;
        case -2:
            print_instruction(Localize("Congratulation, you finished this tutorial.", ""),1);
            print_instruction(Localize("You are now ready to participate to the", ""),2);
            print_instruction(Localize("world challenge !", ""),3);
            print_instruction(Localize("Try to be the best !", ""),4);
            if (!(plyr->control.is_accelerating)) step = -3;
            break;
        case -3:
            print_instruction(Localize("Congratulation, you finished this tutorial.", ""),1);
            print_instruction(Localize("You are now ready to participate to the", ""),2);
            print_instruction(Localize("world challenge !", ""),3);
            print_instruction(Localize("Try to be the best !", ""),4);
            training_abort=True;
            g_game.race_aborted = True;
            set_game_mode( GAME_OVER );
            break;
            						/* Fin du second Tutorial */ 
            break;
            		        			/*    abandon     */
        case -100:
            print_instruction(Localize("You didn't finished this tutorial.", ""),1);
            print_instruction(Localize("You should try again !", ""),2);
            break;
            
        default:
            break;
    }
    
}

void draw_hud_training( player_data_t *plyr )
{
    if (!g_game.practicing) {
        vector_t vel;
        scalar_t speed;
        
        vel = plyr->vel;
        speed = normalize_vector( &vel );
        
        ui_setup_display();
        draw_instructions(plyr);
    }
    //draw_gauge( speed * M_PER_SEC_TO_KM_PER_H, plyr->control.jump_amt );
    
}


void init_starting_tutorial_step(int i){
    is_condition_verified=False;
    training_abort=False;
    pause_for_long_tutorial_explanation=False;
    step = i;
}

