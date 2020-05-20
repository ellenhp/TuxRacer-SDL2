#include "platform.h"
#include "course_load.h"
#include "race_select.h" // for course_price
#include "tuxracer.h"

static int overscan_percent = 0;
static char *player_name = 0;
static bool_t is_on_ouya_val = False;

bool_t is_on_ouya(void)
{
	return is_on_ouya_val;
}

int get_overscan_percent(void)
{
	return overscan_percent;
}

char race_button_text[20];

char *get_race_text(void)
{
	return "Race";
}

char *get_back_text(void)
{
	if (is_on_ouya_val)
	{
		return OUYA_A_BUTTON "Back";
	}
	else
	{
		return "Back";
	}
}

char *get_continue_text(void)
{
	if (is_on_ouya_val)
	{
		return OUYA_O_BUTTON "Continue";
	}
	else
	{
		return "Continue";
	}
}

char *get_abort_text(void)
{
	if (is_on_ouya_val)
	{
		return OUYA_A_BUTTON "Exit Race";
	}
	else
	{
		return "Exit Race";
	}
}

char *get_select_text(void)
{
	if (is_on_ouya_val)
	{
		return OUYA_O_BUTTON "Select";
	}
	else
	{
		return "Select";
	}
}

char *get_quit_text(void)
{
	if (is_on_ouya_val)
	{
		return OUYA_A_BUTTON "Quit";
	}
	else
	{
		return "Quit";
	}
}
