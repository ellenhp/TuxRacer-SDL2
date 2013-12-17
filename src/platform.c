#include "platform.h"
#include "tuxracer.h"

#ifdef __ANDROID__
#include <jni.h>
#endif

static int overscan_percent=0;
static char* player_name=0;
static bool_t is_on_ouya=False;

static char* course_names[]=
{
	"bunny_hill",
	"frozen_river",
	"twisty_slope",
	"bumpy_ride",
	"penguins_cant_fly",
	"slippy_slidey",
	"chinese_wall",
	"bobsled_ride",
	"deadman",
	"ski_jump",
	"Half_Pipe",
	"Off_Piste_Skiing",
	"in_search_of_vodka",
};

#ifdef __ANDROID__
JNIEXPORT jdouble JNICALL Java_com_moonlite_tuxracer_SDLActivity_nativeSetPlayerData
(JNIEnv * env, jobject jobj, jstring path, jboolean on_ouya)
{
	char* name = (*env)->GetStringUTFChars(env, path , 0);
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

int get_course_index(char* course_name)
{
	int i;
	for (i=0; i<sizeof(course_names)/sizeof(course_names[0]); i++)
	{
		if (strcmp(course_names[i], course_name)==0)
		{
			return i;
		}
	}
	return -1;
}

int get_num_courses()
{
	return sizeof(course_names)/sizeof(char*);
}

void submit_score(char* course_name, int score)
{
	int course=get_course_index(course_name);
#ifdef __ANDROID__
	JNIEnv* env = Android_JNI_GetEnv();
    if (!env) {
        return;
    }
    
	jclass mActivityClass = (*env)->FindClass(env, "com/moonlite/tuxracer/SDLActivity");

    jmethodID mid = (*env)->GetStaticMethodID(env, mActivityClass, "PostScore", "(II)V");
    if (!mid) {
        return ;
    }
    
	(*env)->CallStaticVoidMethod(env, mActivityClass, mid, course+1, score);
#else
	print_debug(DEBUG_OTHER, "");
#endif
}

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