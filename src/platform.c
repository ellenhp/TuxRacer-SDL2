#include "platform.h"
#include "tuxracer.h"

#ifdef __ANDROID__
#include <jni.h>
#endif

static int overscan_percent=0;
static char* player_name=0;
static bool_t is_on_ouya=False;

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
		is_on_ouya=True;
		overscan_percent=5;
	}
	else
	{
		overscan_percent=0;
	}
}
#endif

int get_overscan_percent()
{
	return overscan_percent;
}

char* get_race_text()
{
	if (is_on_ouya)
	{
		return "\x01 Race";
	}
	else
	{
		return "Race";
	}
}

char* get_back_text()
{
	if (is_on_ouya)
	{
		return "\x04 Back";
	}
	else
	{
		return "Back";
	}
}
