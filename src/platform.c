#include "platform.h"
#include "course_load.h"
#include "tuxracer.h"

#ifdef __ANDROID__
#include <jni.h>
#endif

static int overscan_percent=0;
static char* player_name=0;
static bool_t is_on_ouya_val=False;

#ifdef __ANDROID__
JNIEXPORT jdouble JNICALL Java_com_moonlite_tuxracer_SDLActivity_nativeSetPlayerData
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

bool_t is_on_ouya()
{
    return is_on_ouya_val;
}

int get_overscan_percent()
{
	return overscan_percent;
}

float course_price = 2.99;
char race_button_text[20];

char* get_race_text()
{
	const char* button_format;
	const char* course_name = get_current_course_name();

	if (!is_course_free(course_name) && (course_price != 0.0))
	{
		button_format = "Buy $%0.2f";
	}
	else
	{
		button_format = "Race";
	}

	if (is_on_ouya_val)
	{
		strcpy(race_button_text, "'\x01 ");
		sprintf(race_button_text+2, button_format, course_price);
	}
	else
	{
		sprintf(race_button_text, button_format, course_price);
	}
	return race_button_text;
}

char* get_back_text()
{
	if (is_on_ouya_val)
	{
		return "\x04 Back";
	}
	else
	{
		return "Back";
	}
}

char* get_continue_text()
{
	if (is_on_ouya_val)
	{
		return "\x01 Continue";
	}
	else
	{
		return "Continue";
	}
}

char* get_abort_text()
{
	if (is_on_ouya_val)
	{
		return "\x04 Exit Race";
	}
	else
	{
		return "Exit Race";
	}
}

char* get_select_text()
{
	if (is_on_ouya_val)
	{
		return "\x01 Select";
	}
	else
	{
		return "Select";
	}
}

char* get_quit_text()
{
	if (is_on_ouya_val)
	{
		return "\x04 Quit";
	}
	else
	{
		return "Quit";
	}
}
