#include "gui_input.h"
#include "gui_mgr.h"
#include "gui_abstraction.h"
#include <math.h>
#include <limits.h>

#include "SDL.h"
#include "SDL_gamecontroller.h"

#define JOYSTICK_TRIGGER 0.5
#define JOYSTICK_RELAX 0.35

#define JOYSTICK_REPEAT_DELAY 300
#define JOYSTICK_REPEAT_RATE_INVERSE 150

int joystick_x_status=0, joystick_y_status=0;
int joystick_x_trigger_time=INT_MAX, joystick_y_trigger_time=INT_MAX;
int joystick_x_repeating=0, joystick_y_repeating=0;

double joystick_x=0, joystick_y=0;

int select_button=-1; //if uninitialized, this won't ever match a real button

void gui_process_keypress(int key)
{
	input_t input=NONE_INPUT;
	switch (key)
	{
	case SDLK_DOWN:
		input=DOWN_INPUT;
		break;
	case SDLK_UP:
		input=UP_INPUT;
		break;
	case SDLK_RIGHT:
		input=RIGHT_INPUT;
		break;
	case SDLK_LEFT:
		input=LEFT_INPUT;
		break;
	case SDLK_RETURN:
	case SDLK_SPACE:
		input=SELECT_INPUT;
		break;
	}
	gui_process_input(input);
}

void gui_process_button_press(int button)
{
	input_t input=NONE_INPUT;
	switch (button)
	{
	case SDL_CONTROLLER_BUTTON_A:
		input=SELECT_INPUT;
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		input=UP_INPUT;
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		input=DOWN_INPUT;
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		input=LEFT_INPUT;
		break;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		input=RIGHT_INPUT;
		break;
	}
	gui_process_input(input);
}

void gui_set_select_button(int button)
{
	
}

void gui_process_axis(double x, double y)
{
	joystick_x=x;
	joystick_y=y;
}

void gui_input_update()
{
	if (joystick_x_status!=0)
	{
		if (fabs(joystick_x)<JOYSTICK_RELAX)
		{
			joystick_x_status=0;
			joystick_x_repeating=0;
			joystick_x_trigger_time=INT_MAX;
		}
		else if (joystick_x_repeating && SDL_GetTicks()-joystick_x_trigger_time>JOYSTICK_REPEAT_RATE_INVERSE)
		{
			joystick_x_trigger_time=SDL_GetTicks();
			if (joystick_x_status==-1)
			{
				gui_process_input(LEFT_INPUT);
			}
			else if (joystick_x_status==1)
			{
				gui_process_input(RIGHT_INPUT);
			}
		}
		else if (SDL_GetTicks()-joystick_x_trigger_time>JOYSTICK_REPEAT_DELAY)
		{
			joystick_x_repeating=1;
			joystick_x_trigger_time=SDL_GetTicks();
			if (joystick_x_status==-1)
			{
				gui_process_input(LEFT_INPUT);
			}
			else if (joystick_x_status==1)
			{
				gui_process_input(RIGHT_INPUT);
			}
		}
	}
	else
	{
		if (joystick_x<-JOYSTICK_TRIGGER)
		{
			joystick_x_status=-1;
			joystick_x_trigger_time=SDL_GetTicks();
			gui_process_input(LEFT_INPUT);
		}
		if (joystick_x>JOYSTICK_TRIGGER)
		{
			joystick_x_status=1;
			joystick_x_trigger_time=SDL_GetTicks();
			gui_process_input(RIGHT_INPUT);
		}
	}

	if (joystick_y_status!=0)
	{
		if (fabs(joystick_y)<JOYSTICK_RELAX)
		{
			joystick_y_status=0;
			joystick_y_repeating=0;
			joystick_y_trigger_time=INT_MAX;
		}
		else if (joystick_y_repeating && SDL_GetTicks()-joystick_y_trigger_time>JOYSTICK_REPEAT_RATE_INVERSE)
		{
			joystick_y_trigger_time=SDL_GetTicks();
			if (joystick_y_status==-1)
			{
				gui_process_input(UP_INPUT);
			}
			else if (joystick_y_status==1)
			{
				gui_process_input(DOWN_INPUT);
			}
		}
		else if (SDL_GetTicks()-joystick_y_trigger_time>JOYSTICK_REPEAT_DELAY)
		{
			joystick_y_repeating=1;
			joystick_y_trigger_time=SDL_GetTicks();
			if (joystick_y_status==-1)
			{
				gui_process_input(UP_INPUT);
			}
			else if (joystick_y_status==1)
			{
				gui_process_input(DOWN_INPUT);
			}
		}
	}
	else
	{
		if (joystick_y<-JOYSTICK_TRIGGER)
		{
			joystick_y_status=-1;
			joystick_y_trigger_time=SDL_GetTicks();
			gui_process_input(UP_INPUT);
		}
		if (joystick_y>JOYSTICK_TRIGGER)
		{
			joystick_y_status=1;
			joystick_y_trigger_time=SDL_GetTicks();
			gui_process_input(DOWN_INPUT);
		}
	}
}

