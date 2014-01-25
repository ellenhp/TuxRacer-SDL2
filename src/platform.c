#include "platform.h"
#include "course_load.h"
#include "race_select.h"		// for course_price
#include "tuxracer.h"

#ifdef __ANDROID__
#include <jni.h>
#endif

static int overscan_percent=0;
static char* player_name=0;
static bool_t is_on_ouya_val=False;

#ifdef __ANDROID__

#define JNI(f)	Java_com_moonlite_tuxracer_ ## f

JNIEXPORT jdouble JNICALL JNI(SDLActivity_nativeSetPlayerData)
(JNIEnv * env, jobject jobj, jstring path, jboolean on_ouya)
{
	const char* name = (*env)->GetStringUTFChars(env, path , 0);
	if (player_name)
	{
		free(player_name);
	}
	player_name=(char*)malloc(strlen(name));
	strcpy(player_name, name);
	
	if (on_ouya)
	{
		is_on_ouya_val=True;
		overscan_percent=5;
	}
	else
	{
		overscan_percent=0;
	}
}
#endif

bool_t is_on_ouya(void)
{
    return is_on_ouya_val;
}

int get_overscan_percent(void)
{
	return overscan_percent;
}

char race_button_text[20];

char* get_race_text(void)
{
	const char* button_format;

	if (buy_or_play_course())
	{
		button_format = "Buy $%0.2f";
	}
	else
	{
		button_format = "Race";
	}

	if (is_on_ouya_val)
	{
		strcpy(race_button_text, OUYA_O_BUTTON);
		sprintf(race_button_text+2, button_format, course_price);
	}
	else
	{
		sprintf(race_button_text, button_format, course_price);
	}
	return race_button_text;
}

char* get_back_text(void)
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

char* get_continue_text(void)
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

char* get_abort_text(void)
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

char* get_select_text(void)
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

char* get_quit_text(void)
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
