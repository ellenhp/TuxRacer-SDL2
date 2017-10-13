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
#include "course_load.h"
#include "course_render.h"
#include "textures.h"
#include "fonts.h"
#include "tux.h"
#include "phys_sim.h"
#include "part_sys.h"
#include "keyframe.h"
#include "preview.h"
#include "gl_util.h"
#include "game_config.h"
#include "loop.h"
#include "render_util.h"
#include "intro.h"
#include "racing.h"
#include "game_over.h"
#include "paused.h"
#include "reset.h"
#include "keyboard.h"
#include "multiplayer.h"
#include "audio_data.h"
#include "audio.h"
#include "ui_mgr.h"
#include "course_mgr.h"
#include "game_type_select.h"
#ifdef SPEED_MODE
    #include "racing_mode_select.h"
#endif
#include "race_select.h"
#include "event_select.h"
#include "save.h"
#include "credits.h"
#include "prefs.h"
#include "joystick.h"
#include "os_util.h"
#include "loading.h"
#include "tcl_util.h"
#include "tcl.h"
#ifdef TARGET_OS_IPHONE
    #include "sharedGeneralFunctions.h"
#endif

//#define NV_PROFILE
#ifdef NV_PROFILE
#include "EGL/egl.h"
#include "EGL/eglext.h"
#endif

#define WINDOW_TITLE "Tux Racer " VERSION

#define GAME_INIT_SCRIPT "tuxracer_init.tcl"
#define COURSE_INDEX_SCRIPT "courses/course_idx.tcl"

#define SCRIPT_MAX_SIZE 20000

#ifndef MAX_PATH
#  ifdef PATH_MAX
#    define MAX_PATH PATH_MAX
#  else
#    define MAX_PATH 8192 /* this ought to be more than enough */
#  endif
#endif

/*
 * Globals 
 */

game_data_t g_game;


/* 
 * Function definitions
 */


/* This function is called on exit */
void cleanup(void)
{
    write_config_file();
    write_saved_games();

    shutdown_audio();

    winsys_shutdown();
}

int tux_eval(Tcl_Interp *interp, char* filename)
{
    char tcl_script_buf[SCRIPT_MAX_SIZE];
    int bytes_read=0;
    SDL_RWops* file=0;
    
    file=SDL_RWFromFile(filename, "r");
    if (!file)
    {
        handle_error( 1, "error opening script %s", filename);
        return 0;
    }
    bytes_read=SDL_RWread(file, tcl_script_buf, 1, SCRIPT_MAX_SIZE-1);
    tcl_script_buf[bytes_read]=0; //null terminate it!
    SDL_RWclose(file);
    if ( Tcl_Eval( interp, tcl_script_buf) == TCL_ERROR )
    {
        handle_error(1, "error evalating %s: %s", filename, Tcl_GetStringResult( g_game.tcl_interp));
        return 0;
    }
    return 1;
}

int tcl_tux_eval(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    if (argc!=2)
    {
        Tcl_SetResult(interp, "Inappropriate arguments for tux_eval", 0);
        return TCL_ERROR;
    }
    if (tux_eval(interp, argv[1]))
    {
        Tcl_SetResult(interp, "tux_eval evaluated file", 0);
        return TCL_OK;
    }
    Tcl_SetResult(interp, "Error evaluating script", 0);
    return TCL_ERROR;
}

void read_game_init_script()
{
    char cwd[BUFF_LEN];
    
    char tcl_script_buf[SCRIPT_MAX_SIZE];
    int bytes_read=0;
    SDL_RWops* file=0;

    Tcl_CreateCommand(g_game.tcl_interp, "tux_eval", (Tcl_CmdProc*)tcl_tux_eval, 0, NULL);
    
    tux_eval(g_game.tcl_interp, GAME_INIT_SCRIPT);
    
    print_debug(DEBUG_OTHER, "Read game init script");

    check_assertion( !Tcl_InterpDeleted( g_game.tcl_interp ),
		     "Tcl interpreter deleted" );
    
    /*
    if ( chdir( cwd ) != 0 ) {
	handle_system_error( 1, "couldn't chdir to %s", cwd );
    }
    */
}

    

#ifdef TARGET_OS_IPHONE
int libtuxracer_main( int argc, char **argv )
#elif defined(__ANDROID__)
int SDL_main( int argc, char **argv )
#else
int main( int argc, char **argv ) 
#endif
{
    char curdir[MAX_PATH];
    /* Print copyright notice */

    print_debug(DEBUG_OTHER, 
         "Tux Racer " VERSION " -- a Sunspire Studios Production "
	     "(http://www.sunspirestudios.com)\n"
	     "(c) 1999-2000 Jasmin F. Patry "
	     "<jfpatry@sunspirestudios.com>\n"
	     "\"Tux Racer\" is a trademark of Jasmin F. Patry\n"
	     "Tux Racer comes with ABSOLUTELY NO WARRANTY. "
	     "This is free software,\nand you are welcome to redistribute "
	     "it under certain conditions.\n"
	     "See http://www.gnu.org/copyleft/gpl.html for details.\n\n" );

    /* Init the game clock */
    g_game.secs_since_start = 0;

    /* Seed the random number generator */
    srand( time(NULL) );


    /*
     * Set up the game configuration
     */

    /* Don't support multiplayer, yet... */
#ifdef SPEED_TRIAL
    g_game.num_players = 2;
#else
    g_game.num_players = 2;
#endif

    
    /* Create a Tcl interpreter */
    g_game.tcl_interp = Tcl_CreateInterp();

    if ( g_game.tcl_interp == NULL ) {
	handle_error( 1, "cannot create Tcl interpreter" ); 
    }

    /* Setup the configuration variables and read the ~/.tuxracer/options file */
    init_game_configuration();
    read_config_file();

    /* Set up the debugging modes */
    init_debug();

    /* Setup diagnostic log if requested */
    if ( getparam_write_diagnostic_log() ) {
	setup_diagnostic_log();
    }

    /*
     * Setup Tcl stdout and stderr channels to point to C stdout and stderr 
     * streams
     */
    setup_tcl_std_channels();


    /* 
     * Initialize rendering context, create window
     */
    winsys_init( &argc, argv, WINDOW_TITLE, WINDOW_TITLE );

#if SDL_MAJOR_VERSION==1
    /* Ingore key-repeat messages */
    winsys_enable_key_repeat(0);
#endif

    /* Set up a function to clean up when program exits */
    winsys_atexit( cleanup );

    /*
     * Initial OpenGL settings 
     */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    init_opengl_extensions();
    
#ifdef NV_PROFILE
    PFNEGLGETSYSTEMTIMENVPROC eglGetSystemTimeNV;
    eglGetSystemTimeNV = (PFNEGLGETSYSTEMTIMENVPROC) eglGetProcAddress("eglGetSystemTimeNV");
    
    eglGetSystemTimeNV();
#endif
    
    /* Print OpenGL debugging information if requested */
    if ( debug_mode_is_active( DEBUG_GL_INFO ) ) {
	print_debug( DEBUG_GL_INFO, 
		     "OpenGL information:" );
	print_gl_info();
    }

    /* 
     * Load the game data and initialize game state
     */
    register_game_config_callbacks( g_game.tcl_interp );
    register_course_load_tcl_callbacks( g_game.tcl_interp );
    register_key_frame_callbacks( g_game.tcl_interp );
    register_particle_callbacks( g_game.tcl_interp );
    register_texture_callbacks( g_game.tcl_interp );
    register_font_callbacks( g_game.tcl_interp );
    register_sound_tcl_callbacks( g_game.tcl_interp );
    register_sound_data_tcl_callbacks( g_game.tcl_interp );
    register_course_manager_callbacks( g_game.tcl_interp );

	// Preserve current directory (changed inside init_saved_games!)
	getcwd( curdir, BUFF_LEN - 1 );
    init_saved_games();
    chdir( curdir );
    load_tux();
    init_textures();
    init_fonts();
    init_audio_data();
    init_audio();
    init_ui_manager();
    init_course_manager();
    
    /* Read the tuxracer_init.tcl file */
    read_game_init_script();

    /* Need to set up an initial view position for select_course 
       (quadtree simplification)
    */
    
    //Player 0 = classic mode player ; Player 1 = Speed Only Mode Player
    
    g_game.player[0].view.pos = make_point( 0., 0., 0. );
#ifdef SPEED_MODE
    g_game.player[1].view.pos = make_point( 0., 0., 0. );
#endif    

    /* Placeholder name until we give players way to enter name */
    g_game.player[0].name = "noname";
#ifdef SPEED_MODE
    g_game.player[1].name = "nonameSpeedOnly";
#endif   

    init_preview();

    intro_register();
    racing_register();
    game_over_register();
    paused_register();
    reset_register();
    game_type_select_register();
#ifdef SPEED_MODE
    racing_mode_select_register();
#endif
    event_select_register();
    race_select_register();
    credits_register();
	prefs_register();
    loading_register();

    g_game.mode = NO_MODE;
    
    init_ui_snow();
    set_game_mode( GAME_TYPE_SELECT );

    g_game.difficulty = DIFFICULTY_LEVEL_NORMAL;

    init_keyboard();
    
    init_scoreboard_arrays();
    
	/* Hide system cursor (arrow) we draw custom cursor */
    winsys_show_cursor( False );

    /* We use this to "prime" the GLUT loop */
    winsys_set_idle_func( main_loop );

    
    /* 
     * ...and off we go!
     */
    winsys_process_events();

    return 0;
} 

