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
#include "winsys.h"
#include "audio.h"
#include "SDL_image.h"
#include "shaders.h"

/* Windowing System Abstraction Layer */
/* Abstracts creation of windows, handling of events, etc. */

#if defined( HAVE_SDL )

#if defined( HAVE_SDL_MIXER )
#   include "SDL_mixer.h"
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* SDL version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#if SDL_MAJOR_VERSION==2
static SDL_Window *window = NULL;
#else
static SDL_Surface *screen = NULL;
#endif


static winsys_display_func_t display_func = NULL;
static winsys_idle_func_t idle_func = NULL;
static winsys_reshape_func_t reshape_func = NULL;
static winsys_keyboard_func_t keyboard_func = NULL;
static winsys_mouse_func_t mouse_func = NULL;
static winsys_motion_func_t motion_func = NULL;
static winsys_motion_func_t passive_motion_func = NULL;
static winsys_joystick_func_t joystick_func = NULL;
static winsys_joystick_button_func_t joystick_button_func = NULL;

static winsys_atexit_func_t atexit_func = NULL;

static bool_t redisplay = False;

double x_joystick=0, y_joystick=0;

#define JOYSTICK_TRIGGER 0.5
#define JOYSTICK_RELAX 0.35

#define JOYSTICK_REPEAT_DELAY 400
#define JOYSTICK_REPEAT_RATE_INVERSE 200

#define MAX_JS_BINDINGS 10

int winsys_joystick_x_status=0, winsys_joystick_y_status=0;
int winsys_joystick_x_trigger_time=INT_MAX, winsys_joystick_y_trigger_time=INT_MAX;
int winsys_joystick_x_repeating=0, winsys_joystick_y_repeating=0;

static SDL_Joystick *winsys_joystick = NULL;
static SDL_GameController* winsys_game_controller=NULL;
static int winsys_num_buttons = 0;
static int winsys_num_axes = 0;

int num_js_bindings=0;
js_binding_t js_bindings[MAX_JS_BINDINGS];

static Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;

bool_t bind_js_axes=False;

static bool_t winsys_drawing_allowed=True;

/*---------------------------------------------------------------------------*/
/*!
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay()
{
    redisplay = True;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the display callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_display_func( winsys_display_func_t func )
{
    display_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the idle callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_idle_func( winsys_idle_func_t func )
{
    idle_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the reshape callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    reshape_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the keyboard callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse button-press callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    mouse_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse motion callback (when a mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_motion_func( winsys_motion_func_t func )
{
    motion_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse motion callback (when no mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    passive_motion_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
  Sets the joystick callback
  \author  nopoe
  \date    Created:  2013-10-06
  \date    Modfiied: 2013-10-06
*/
void winsys_set_joystick_func( winsys_joystick_func_t func )
{
    joystick_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
  Sets the joystick button callback
  \author  nopoe
  \date    Created:  2013-10-06
  \date    Modfiied: 2013-10-06
*/
void winsys_set_joystick_button_func( winsys_joystick_button_func_t func )
{
    joystick_button_func = func;
}



/*---------------------------------------------------------------------------*/
/*!
  Copies the OpenGL back buffer to the front buffer
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_swap_buffers()
{
#if SDL_MAJOR_VERSION==2
    SDL_GL_SwapWindow(window);
#else
    SDL_GL_SwapBuffers();
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Moves the mouse pointer to (x,y)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_warp_pointer( int x, int y )
{
#if SDL_MAJOR_VERSION==2
    SDL_WarpMouseInWindow( window, x, y );
#else
	SDL_WarpMouse( x, y );
#endif
}


/*---------------------------------------------------------------------------*/
/*!
  Sets up the SDL OpenGL rendering context
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
static float scale_factor = 1.0;

static void setup_sdl_video_mode()
{
#if SDL_MAJOR_VERSION==2
    Uint32 video_flags = SDL_WINDOW_OPENGL;
#else
    Uint32 video_flags = SDL_OPENGL;
#endif
    int bpp = 0;
    int width, height;

#if SDL_MAJOR_VERSION==2

    if ( getparam_fullscreen() )
    {
        video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        width = 0;
        height = 0;
    }
    else
    {
        video_flags |= SDL_WINDOW_RESIZABLE;
        width = getparam_x_resolution();
        height = getparam_y_resolution();
    }

#else
	if ( getparam_fullscreen() ) {
	video_flags |= SDL_FULLSCREEN;
    } else {
	video_flags |= SDL_RESIZABLE;
    }

    width = getparam_x_resolution();
    height = getparam_y_resolution();

    switch ( getparam_bpp_mode() ) {
    case 0:
	/* Use current bpp */
	bpp = 0;
	break;

    case 1:
	/* 16 bpp */
	bpp = 16;
	break;

    case 2:
	/* 32 bpp */
	bpp = 32;
	break;

    default:
	setparam_bpp_mode( 0 );
	bpp = getparam_bpp_mode();
    }
#endif

#if SDL_MAJOR_VERSION==2
    if ( ( window = SDL_CreateWindow( "Tux Racer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, video_flags ) ) == NULL )
#else
	if ( ( screen = SDL_SetVideoMode( width, height, bpp, video_flags ) ) == NULL )
#endif
    {
	handle_system_error( 1, "Couldn't initialize video: %s",
			     SDL_GetError() );
    }
#if SDL_MAJOR_VERSION==2
	else
	{
        SDL_GetWindowSize(window, &width, &height);
		print_debug(DEBUG_OTHER, "width=%d height=%d", width, height);
		/* Set scale factor for fonts & HUD */
		scale_factor = height / 480.0;
        setparam_x_resolution(width);
        setparam_y_resolution(height);
		SDL_GL_CreateContext(window);
	}
#endif
}

/*---------------------------------------------------------------------------*/
/*!
  Handles time-critical events on mobile devices.
  \author  nopoe
  \date    Created:  2013-10-08
  \date    Modified: 2013-10-08 */
int winsys_event_filter(void* userdata, SDL_Event* event)
{
	switch (event->type)
	{
    case SDL_APP_WILLENTERBACKGROUND:
        mute_audio();
        winsys_drawing_allowed=False;
        return 0;
	case SDL_APP_WILLENTERFOREGROUND:
		init_ui_snow();
        unmute_audio();
        winsys_drawing_allowed=True;
		return 0;
	case SDL_APP_TERMINATING:
		winsys_shutdown();
		return 1;
	}
	return 1;
}


/*---------------------------------------------------------------------------*/
/*!
  Initializes the OpenGL rendering context, and creates a window (or
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_init( int *argc, char **argv, char *window_title,
		  char *icon_title )
{
	int i;
	int initResult;
    /*
     * Initialize SDL
     */
    if ( SDL_Init( sdl_flags ) < 0 ) {
	handle_error( 1, "Couldn't initialize SDL: %s", SDL_GetError() );
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	initResult=IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );
    if (!(initResult & IMG_INIT_PNG) || !(initResult & IMG_INIT_JPG)) {
	handle_error( 1, "Couldn't initialize SDL_image: %s", IMG_GetError() );
    }


    /*
     * Init video
     */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

#if defined( USE_STENCIL_BUFFER )
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
#endif

    setup_sdl_video_mode();

    init_programs();

#if SDL_MAJOR_VERSION==2
	SDL_SetWindowTitle(window, window_title);
#else
    SDL_WM_SetCaption( window_title, icon_title );
#endif
	winsys_init_joystick();
	SDL_SetEventFilter(winsys_event_filter, NULL);
}

float winsys_scale(float x)
{
	return x * scale_factor;
}

void winsys_scan_joysticks()
{
    int num_joysticks = 0;
    char *js_name;
	char guid[64];
	int js_index=0;
	int controllerIndex=0;
	int i=0;

	if (winsys_joystick)
	{
		SDL_JoystickClose(winsys_joystick);
		winsys_joystick = NULL;
	}
	if (winsys_game_controller)
	{
		SDL_GameControllerClose(winsys_game_controller);
		winsys_game_controller = NULL;
	}

	num_joysticks = SDL_NumJoysticks();

  print_debug( DEBUG_JOYSTICK, "Found %d joysticks", num_joysticks );

  if ( num_joysticks == 0 )
	{
		return;
  }

	for (controllerIndex=0; controllerIndex<num_joysticks; controllerIndex++)
	{
    winsys_joystick = SDL_JoystickOpen(controllerIndex);

  	if (winsys_joystick == NULL)
  	{
  		print_debug( DEBUG_JOYSTICK, "Cannot open joystick" );
  		return;
  	}

  	js_name = (char*) SDL_JoystickName(winsys_joystick);
  	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(winsys_joystick), guid, sizeof(guid));

  	winsys_num_buttons = SDL_JoystickNumButtons(winsys_joystick);
  	print_debug( DEBUG_JOYSTICK, "Joystick has %d button%s",
  		 winsys_num_buttons, winsys_num_buttons == 1 ? "" : "s" );

  	/* Get number of axes */
  	winsys_num_axes = SDL_JoystickNumAxes(winsys_joystick);
  	print_debug( DEBUG_JOYSTICK, "Joystick has %d ax%ss",
  		 winsys_num_axes, winsys_num_axes == 1 ? "i" : "e" );

  	if (winsys_num_axes==3)
  	{
  		print_debug(DEBUG_JOYSTICK, "Using accelerometer");
  		SDL_JoystickEventState(SDL_ENABLE);
  	}
  	else
  	{
      // char mapping_buffer[200];
      // print_debug(DEBUG_JOYSTICK, "Incompatible joystick '%s' with GUID '%s'", js_name, guid);
      // print_debug(DEBUG_JOYSTICK, "Creating best-guess mapping for joystick '%s' with GUID '%s'", js_name, guid);
      // sprintf(mapping_buffer, "%s,Unrecognized Joystick,a:b5,b:b6,x:b8,y:b9,dpup:b0,dpleft:b2,dpdown:b1,dpright:b3,leftx:a0,lefty:a1,");
      // SDL_GameControllerAddMapping(mapping_buffer);
      SDL_JoystickClose(winsys_joystick);
      winsys_joystick=NULL;
  	}
	}

}

void winsys_init_joystick()
{

	SDL_GameControllerAddMapping("8e06f600000000000000504944564944,CH FLIGHTSTICK PRO,a:b1,b:b3,leftx:a0,lefty:a1,x:b0,y:b2,");
	SDL_GameControllerAddMapping("8e06f400000000000000504944564944,CH COMBATSTICK,a:b6,b:b5,y:b4,x:b7,start:b9,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,");
	winsys_scan_joysticks();
}

bool_t winsys_is_joystick_active()
{
	return (bool_t)(winsys_game_controller!=NULL || winsys_joystick!=NULL);
}

bool_t winsys_is_controller_active()
{
	return (bool_t)(winsys_game_controller!=NULL);
}

/*---------------------------------------------------------------------------*/
/*!
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
	IMG_Quit();
    SDL_QuitSubSystem(sdl_flags);
}


/*---------------------------------------------------------------------------*/
/*!
  Enables/disables key repeat messages from being generated
  \return
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
#if SDL_MAJOR_VERSION==1
void winsys_enable_key_repeat( bool_t enabled )
{
    if ( enabled ) {
	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY,
			     SDL_DEFAULT_REPEAT_INTERVAL );
    } else {
	SDL_EnableKeyRepeat( 0, 0 );
    }
}
#endif


/*---------------------------------------------------------------------------*/
/*!
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool_t visible )
{
    SDL_ShowCursor( visible );
}

/*---------------------------------------------------------------------------*/
/*!
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_process_events()
{
	int z;
    SDL_Event event;
    unsigned int key;
    int x, y;
	int i;

    while (True) {

	SDL_LockAudio();
	SDL_UnlockAudio();

	while ( SDL_PollEvent( &event ) ) {

	    switch ( event.type ) {
	    case SDL_KEYDOWN:
		if ( keyboard_func ) {
		    SDL_GetMouseState( &x, &y );
		    key = event.key.keysym.sym;
		    (*keyboard_func)( key,
				      key >= 256,
				      False,
				      x, y );
		}
		break;

	    case SDL_KEYUP:
		if ( keyboard_func ) {
		    SDL_GetMouseState( &x, &y );
		    key = event.key.keysym.sym;
		    (*keyboard_func)( key,
				      key >= 256,
				      True,
				      x, y );
		}
		break;

		case SDL_JOYAXISMOTION:
			if (!winsys_game_controller && event.jaxis.axis==getparam_joystick_x_axis())
			{
				//accelerometer
				x_joystick=2*event.jaxis.value/32767.0;
				if (x_joystick>1)
					x_joystick=1;
				if (x_joystick<-1)
					x_joystick=-1;
				if (joystick_func)
				{
					joystick_func(-x_joystick, 0);
				}
			}
			break;

		case SDL_CONTROLLERAXISMOTION:
			if (event.caxis.axis==SDL_CONTROLLER_AXIS_LEFTX)
			{
				x_joystick=event.caxis.value/32767.0;
			}
			else if (event.caxis.axis==SDL_CONTROLLER_AXIS_LEFTY)
			{
				y_joystick=event.caxis.value/32767.0;
			}
			if (joystick_func)
			{
				joystick_func(x_joystick, y_joystick);
			}
			break;

		case SDL_CONTROLLERBUTTONDOWN:
			if (joystick_button_func)
			{
				joystick_button_func(event.cbutton.button);
			}
			if (!keyboard_func)
			{
				break;
			}
			for (i=0; i<num_js_bindings; i++)
			{
				if (js_bindings[i].js_button==event.cbutton.button)
				{
					keyboard_func(js_bindings[i].sdl_key, js_bindings[i].sdl_key>255, False, 0, 0);
				}
			}
			break;

		case SDL_CONTROLLERBUTTONUP:
			if (!keyboard_func)
			{
				break;
			}
			for (i=0; i<num_js_bindings; i++)
			{
				if (js_bindings[i].js_button==event.cbutton.button)
				{
					keyboard_func(js_bindings[i].sdl_key, js_bindings[i].sdl_key>255, True, 0, 0);
				}
			}
			break;
			break;

	    case SDL_MOUSEBUTTONDOWN:
	    case SDL_MOUSEBUTTONUP:

		if (SDL_GetNumTouchDevices()>0)
		{
			break;
		}

		if ( mouse_func ) {
		    (*mouse_func)( event.button.button,
				   event.button.state,
				   -1,
				   event.button.x,
				   event.button.y );
		}
		break;

	    case SDL_FINGERDOWN:

		if ( mouse_func ) {
		    (*mouse_func)( SDL_BUTTON_LEFT,
				   SDL_PRESSED,
				   event.tfinger.fingerId,
				   (int)(event.tfinger.x*getparam_x_resolution()),
				   (int)(event.tfinger.y*getparam_y_resolution()) );
		}
		break;

	    case SDL_FINGERUP:

		if ( mouse_func ) {
		    (*mouse_func)( SDL_BUTTON_LEFT,
				   SDL_RELEASED,
				   event.tfinger.fingerId,
				   (int)(event.tfinger.x*getparam_x_resolution()),
				   (int)(event.tfinger.y*getparam_y_resolution()) );
		}
		break;

	    case SDL_MOUSEMOTION:
		if (SDL_GetNumTouchDevices()>0)
		{
			break;
		}

		if ( event.motion.state ) {
		    /* buttons are down */
		    if ( motion_func ) {
			(*motion_func)( event.motion.x,
					event.motion.y );
		    }
		} else {
		    /* no buttons are down */
		    if ( passive_motion_func ) {
			(*passive_motion_func)( event.motion.x,
						event.motion.y );
		    }
		}
		break;

		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			winsys_scan_joysticks();
			break;

		case SDL_QUIT:
			winsys_exit(0);
			break;

#if SDL_MAJOR_VERSION==1
	    case SDL_VIDEORESIZE:
		setup_sdl_video_mode();
		if ( reshape_func ) {
		    (*reshape_func)( event.resize.w,
				     event.resize.h );
		}
		break;
#endif
	    }

	    SDL_LockAudio();
	    SDL_UnlockAudio();
	}

	if ( redisplay && display_func ) {
	    redisplay = False;
	    (*display_func)();
	} else if ( idle_func ) {
	    (*idle_func)();
	}

	/* Delay for 1 ms.  This allows the other threads to do some
	   work (otherwise the audio thread gets starved). */
	SDL_Delay(1);

    }

    /* Never exits */
    code_not_reached();
}

/*---------------------------------------------------------------------------*/
/*!
  Adds an assocation between a joystick button and an SDL Key.
  \author  nopoe
  \date    Created:  2013-10-08
  \date    Modified: 2013-10-08 */
void winsys_add_js_button_binding(int js_button, int sdl_key)
{
	if (num_js_bindings<MAX_JS_BINDINGS)
	{
		js_bindings[num_js_bindings].js_button=js_button;
		js_bindings[num_js_bindings].sdl_key=sdl_key;
		num_js_bindings++;
	}
}


/*---------------------------------------------------------------------------*/
/*!
  Binds the joystick axes to the arrow keys
  \author  nopoe
  \date    Created:  2013-10-08
  \date    Modified: 2013-10-08 */
void winsys_add_js_axis_bindings()
{
	bind_js_axes=True;
}


/*---------------------------------------------------------------------------*/
/*!
  Resets all bindings of joystick axes or buttons to keys.
  \author  nopoe
  \date    Created:  2013-10-08
  \date    Modified: 2013-10-08 */
void winsys_reset_js_bindings()
{
	bind_js_axes=False;
	num_js_bindings=0;
}

/*---------------------------------------------------------------------------*/
/*!
  Updates the joysticks for the purposes of sending keyboard events bound
  to the axes.
  \author  nopoe
  \date    Created:  2013-10-08
  \date    Modified: 2013-10-08 */
void winsys_update_joysticks()
{
	if (!keyboard_func || !winsys_game_controller)
	{
		return;
	}
	if (winsys_joystick_x_status!=0)
	{
		if (fabs(x_joystick)<JOYSTICK_RELAX)
		{
			winsys_joystick_x_status=0;
			winsys_joystick_x_repeating=0;
			winsys_joystick_x_trigger_time=INT_MAX;
		}
		else if (winsys_joystick_x_repeating && SDL_GetTicks()-winsys_joystick_x_trigger_time>JOYSTICK_REPEAT_RATE_INVERSE)
		{
			winsys_joystick_x_trigger_time=SDL_GetTicks();
			if (winsys_joystick_x_status==-1)
			{
				(*keyboard_func)(SDLK_LEFT, True, False, 0, 0);
			}
			else if (winsys_joystick_x_status==1)
			{
				(*keyboard_func)(SDLK_RIGHT, True, False, 0, 0);
			}
		}
		else if (SDL_GetTicks()-winsys_joystick_x_trigger_time>JOYSTICK_REPEAT_DELAY)
		{
			winsys_joystick_x_repeating=1;
			winsys_joystick_x_trigger_time=SDL_GetTicks();
			if (winsys_joystick_x_status==-1)
			{
				(*keyboard_func)(SDLK_LEFT, True, False, 0, 0);
			}
			else if (winsys_joystick_x_status==1)
			{
				(*keyboard_func)(SDLK_RIGHT, True, False, 0, 0);
			}
		}
	}
	else
	{
		if (x_joystick<-JOYSTICK_TRIGGER)
		{
			winsys_joystick_x_status=-1;
			winsys_joystick_x_trigger_time=SDL_GetTicks();
			(*keyboard_func)(SDLK_LEFT, True, False, 0, 0);
		}
		if (x_joystick>JOYSTICK_TRIGGER)
		{
			winsys_joystick_x_status=1;
			winsys_joystick_x_trigger_time=SDL_GetTicks();
			(*keyboard_func)(SDLK_RIGHT, True, False, 0, 0);
		}
	}

	if (winsys_joystick_y_status!=0)
	{
		if (fabs(y_joystick)<JOYSTICK_RELAX)
		{
			winsys_joystick_y_status=0;
			winsys_joystick_y_repeating=0;
			winsys_joystick_y_trigger_time=INT_MAX;
		}
		else if (winsys_joystick_y_repeating && SDL_GetTicks()-winsys_joystick_y_trigger_time>JOYSTICK_REPEAT_RATE_INVERSE)
		{
			winsys_joystick_y_trigger_time=SDL_GetTicks();
			if (winsys_joystick_y_status==-1)
			{
				(*keyboard_func)(SDLK_UP, True, False, 0, 0);
			}
			else if (winsys_joystick_y_status==1)
			{
				(*keyboard_func)(SDLK_DOWN, True, False, 0, 0);
			}
		}
		else if (SDL_GetTicks()-winsys_joystick_y_trigger_time>JOYSTICK_REPEAT_DELAY)
		{
			winsys_joystick_y_repeating=1;
			winsys_joystick_y_trigger_time=SDL_GetTicks();
			if (winsys_joystick_y_status==-1)
			{
				(*keyboard_func)(SDLK_UP, True, False, 0, 0);
			}
			else if (winsys_joystick_y_status==1)
			{
				(*keyboard_func)(SDLK_DOWN, True, False, 0, 0);
			}
		}
	}
	else
	{
		if (y_joystick<-JOYSTICK_TRIGGER)
		{
			winsys_joystick_y_status=-1;
			winsys_joystick_y_trigger_time=SDL_GetTicks();
			(*keyboard_func)(SDLK_UP, True, False, 0, 0);
		}
		if (y_joystick>JOYSTICK_TRIGGER)
		{
			winsys_joystick_y_status=1;
			winsys_joystick_y_trigger_time=SDL_GetTicks();
			(*keyboard_func)(SDLK_DOWN, True, False, 0, 0);
		}
	}
}

/*---------------------------------------------------------------------------*/
/*!
  Sets the function to be called when program ends.  Note that this
  function should only be called once.
  \author  jfpatry
  \date    Created:  2000-10-20
  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool_t called = False;

    check_assertion( called == False, "winsys_atexit called twice" );

    called = True;
    atexit_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void winsys_exit( int code )
{
    if ( atexit_func ) {
	(*atexit_func)();
    }
    SDL_DestroyWindow(window);
    exit( code );
}

void winsys_show_preferences( void )
{
}



const char * winsys_localized_string( const char * key, const char * comment )
{
	// TODO: lookup key in localization file
	return key;
}

#elif defined(HAVE_GLUT)

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* GLUT version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static winsys_keyboard_func_t keyboard_func = NULL;

static bool_t redisplay = False;


/*---------------------------------------------------------------------------*/
/*!
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay()
{
    redisplay = True;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the display callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_display_func( winsys_display_func_t func )
{
    glutDisplayFunc( func );
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the idle callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_idle_func( winsys_idle_func_t func )
{
    glutIdleFunc( func );
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the reshape callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    glutReshapeFunc( func );
}


/* Keyboard callbacks */
static void glut_keyboard_cb( unsigned char ch, int x, int y )
{
    if ( keyboard_func ) {
	(*keyboard_func)( ch, False, False, x, y );
    }
}

static void glut_special_cb( int key, int x, int y )
{
    if ( keyboard_func ) {
	(*keyboard_func)( key, True, False, x, y );
    }
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y )
{
    if ( keyboard_func ) {
	(*keyboard_func)( ch, False, True, x, y );
    }
}

static void glut_special_up_cb( int key, int x, int y )
{
    if ( keyboard_func ) {
	(*keyboard_func)( key, True, True, x, y );
    }
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the keyboard callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse button-press callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    glutMouseFunc( func );
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse motion callback (when a mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_motion_func( winsys_motion_func_t func )
{
    glutMotionFunc( func );
}


/*---------------------------------------------------------------------------*/
/*!
  Sets the mouse motion callback (when no mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    glutPassiveMotionFunc( func );
}



/*---------------------------------------------------------------------------*/
/*!
  Copies the OpenGL back buffer to the front buffer
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_swap_buffers()
{
    glutSwapBuffers();
}


/*---------------------------------------------------------------------------*/
/*!
  Moves the mouse pointer to (x,y)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_warp_pointer( int x, int y )
{
    glutWarpPointer( x, y );
}


/*---------------------------------------------------------------------------*/
/*!
  Initializes the OpenGL rendering context, and creates a window (or
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_init( int *argc, char **argv, char *window_title,
		  char *icon_title )
{
    int width, height;
    int glutWindow;

    glutInit( argc, argv );

#ifdef USE_STENCIL_BUFFER
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );
#else
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
#endif

    /* Create a window */
    if ( getparam_fullscreen() ) {
	glutInitWindowPosition( 0, 0 );
	glutEnterGameMode();
    } else {
	/* Set the initial window size */
	width = getparam_x_resolution();
	height = getparam_y_resolution();
	glutInitWindowSize( width, height );

	if ( getparam_force_window_position() ) {
	    glutInitWindowPosition( 0, 0 );
	}

	glutWindow = glutCreateWindow( window_title );

	if ( glutWindow == 0 ) {
	    handle_error( 1, "Couldn't create a window." );
	}
    }
}


/*---------------------------------------------------------------------------*/
/*!
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
    if ( getparam_fullscreen() ) {
	glutLeaveGameMode();
    }
}

/*---------------------------------------------------------------------------*/
/*!
  Enables/disables key repeat messages from being generated
  \return
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_enable_key_repeat( bool_t enabled )
{
    glutIgnoreKeyRepeat(!enabled);
}

/*---------------------------------------------------------------------------*/
/*!
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool_t visible )
{
    if ( visible ) {
	glutSetCursor( GLUT_CURSOR_LEFT_ARROW );
    } else {
	glutSetCursor( GLUT_CURSOR_NONE );
    }
}



/*---------------------------------------------------------------------------*/
/*!
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_process_events()
{
    /* Set up keyboard callbacks */
    glutKeyboardFunc( glut_keyboard_cb );
    glutKeyboardUpFunc( glut_keyboard_up_cb );
    glutSpecialFunc( glut_special_cb );
    glutSpecialUpFunc( glut_special_up_cb );

    glutMainLoop();
}

/*---------------------------------------------------------------------------*/
/*!
  Sets the function to be called when program ends.  Note that this
  function should only be called once.
  \author  jfpatry
  \date    Created:  2000-10-20
  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool_t called = False;

    check_assertion( called == False, "winsys_atexit called twice" );

    called = True;

    atexit(func);
}


/*---------------------------------------------------------------------------*/
/*!
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void winsys_exit( int code )
{
    exit(code);
}


void winsys_show_preferences( void )
{
}

#elif defined(TARGET_OS_IPHONE)

//defined in touchwinsys.m

#else

#error Not supported yet!

#endif /* defined( HAVE_SDL ) */

/* EOF */
