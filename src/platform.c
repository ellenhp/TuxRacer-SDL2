#include "platform.h"

#ifdef __ANDROID__
#include <jni.h>
#endif

static int overscan_percent=0;
static char* player_name=0;

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
		overscan_percent=5;
	}
	else
	{
		overscan_percent=0;
	}
}
#endif

void submit_score(int course, int score)
{
#ifdef __ANDROID__
	JNIEnv* env = Android_JNI_GetEnv();
    if (!env) {
        return;
    }

	jclass mActivityClass = (jclass)((*env)->NewGlobalRef(env, "com.moonlite.SDLActivity"));

    jmethodID mid = (*env)->GetStaticMethodID(env, mActivityClass, "getJoystickName", "(IJ)V");
    if (!mid) {
        return ;
    }
    
	(*env)->CallStaticBooleanMethod(mActivityClass, mid, course, score);
#else

#endif
}

int get_overscan_percent()
{
	return overscan_percent;
}

