#include "scoreboard.h"
#include "tuxracer.h"
#include "gui_label.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __ANDROID__
#include <jni.h>
#endif

char* scoreboard_names[MAX_COURSES][SCOREBOARD_SIZE+1]={NULL};
int scoreboard_scores[MAX_COURSES][SCOREBOARD_SIZE+1]={NULL};

widget_t* rank_labels[SCOREBOARD_SIZE+1]={NULL};
widget_t* name_labels[SCOREBOARD_SIZE+1]={NULL};
widget_t* score_labels[SCOREBOARD_SIZE+1]={NULL};

bool_t scoreboard_open=False;
bool_t arrays_initialized=False;

#ifdef __ANDROID__
JNIEXPORT jdouble JNICALL Java_com_moonlite_tuxracer_SDLActivity_nativeReceivedScores
(JNIEnv * env, jobject jobj, jint course, jobjectArray name_array, jintArray score_array)
{
	jsize len = (*env)->GetArrayLength(env, score_array);
	jint* scores = (*env)->GetIntArrayElements(env, score_array, 0);
	int i;
	jstring tmp_jstring;
	char* name;
	int loop_cutoff=SCOREBOARD_SIZE+1;
	if (len<SCOREBOARD_SIZE+1)
	{
		loop_cutoff=len;
	}
	for (i=0; i<loop_cutoff; i++)
	{
		tmp_jstring=(jstring)((*env)->GetObjectArrayElement(env, name_array, i));
		name=(*env)->GetStringUTFChars(env, tmp_jstring, 0);

		if (scoreboard_names[course][i])
		{
			free(scoreboard_names[course][i]);
		}

		scoreboard_names[course][i]=(char*)malloc(strlen(name)+1);
		strcpy(scoreboard_names[course][i], name);
		scoreboard_names[course][i][strlen(name)]=0;

		(*env)->ReleaseStringUTFChars(env, tmp_jstring, name);

		scoreboard_scores[course][i]=scores[i];
	}
	update_scoreboard_labels();
}
#endif

void refresh_scores()
{
#ifdef __ANDROID__
	JNIEnv* env = Android_JNI_GetEnv();
    if (!env) {
        return;
    }
    
	jclass mActivityClass = (*env)->FindClass(env, "com/moonlite/tuxracer/SDLActivity");
    
    jmethodID mid = (*env)->GetStaticMethodID(env, mActivityClass, "RequestScores", "(I)V");
    if (!mid) {
        return ;
    }
    
	(*env)->CallStaticVoidMethod(env, mActivityClass, mid, 0);

#endif
}

void refresh_scores_for_course(char* course_name)
{

}

void init_scoreboard_labels()
{
	int i;
	char buf[5];
	widget_t* tmp;
    int course, rank;
	for (i=0; i<=SCOREBOARD_SIZE; i++)
	{
		sprintf(buf, "%d", i+1);
		rank_labels[i]=create_label(buf);

		name_labels[i]=create_label("[No Name]");

		score_labels[i]=create_label("0");
	}
	for (course=0; course<=get_num_courses(); course++)
	{
		for (rank=1; rank<=SCOREBOARD_SIZE; rank++)
		{
            scoreboard_names[course][rank]=0;
            scoreboard_scores[course][rank]=0;
		}
	}
}

void update_scoreboard_labels()
{
	char buf[10];
	int course, rank;
    if (!scoreboard_open)
    {
        return;
    }
    course=get_current_course_index();
    for (rank=0; rank<=SCOREBOARD_SIZE; rank++)
    {
        sprintf(buf, "%d", rank+1);
        button_set_text(rank_labels[rank], buf);
        if (scoreboard_names[course][rank])
        {
            button_set_text(name_labels[rank], scoreboard_names[course][rank]);
        }
        else
        {
            button_set_text(name_labels[rank], "----");
        }
        sprintf(buf, "%d", scoreboard_scores[course][rank]);
        button_set_text(score_labels[rank], buf);
    }
}

widget_t* get_name_label(int rank)
{
	return name_labels[rank];
}

widget_t* get_score_label(int rank)
{
	return score_labels[rank];
}

widget_t* get_rank_label(int rank)
{
	return rank_labels[rank];
}

