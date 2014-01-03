#include "scoreboard.h"
#include "course_mgr.h"
#include "tuxracer.h"
#include "gui_label.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_NAME	50

typedef struct score_item
{
	unsigned int rank;
	char name[MAX_NAME];
	unsigned int score;
}
SCORE_ITEM;

/* our (global) scoreboard */
SCORE_ITEM scoreboard[SCOREBOARD_SIZE];

widget_t* rank_labels[SCOREBOARD_SIZE]={NULL};
widget_t* name_labels[SCOREBOARD_SIZE]={NULL};
widget_t* score_labels[SCOREBOARD_SIZE]={NULL};

bool_t scoreboard_open=False;
bool_t arrays_initialized=False;

/* Return 1-based index for a course_name or 0 if not found */
unsigned int get_score_index(const char* course_name)
{
	int course_index = 0;
    list_t course_list=get_score_courses_list();
    list_elem_t cur_elem=get_list_head(course_list);
	while (cur_elem != NULL)
	{
	    open_course_data_t *data = (open_course_data_t*) get_list_elem_data(cur_elem);

		if (strcmp(data->course, course_name) == 0)
		{
			return course_index + 1;
		}
		++course_index;
		cur_elem=(open_course_data_t*) get_next_list_elem(course_list, cur_elem);
	}
	return 0U;
}

#ifdef __ANDROID__

#include <jni.h>
#include <android/log.h>

#define  LOG_TAG    "scoreloop"
#define  LOG(...)   __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define JNI(f)	Java_com_moonlite_tuxracer_ ## f

// Step 2 Include the following Scoreloop header file in your application
#include <scoreloop/scoreloopcore.h>

/* ScoreLoop core variables */
SC_Client_h scoreloopClient;

#if 1
static const char aGameId[] = "d8b0bc95-d65f-4d8d-83ed-9249edd03be3";
static const char aGameSecret[] = "S)TEPoEmjI_]\x7fZf]1C(}INbRP:n>O\\7C1uo|okrl@=BjUXX#8M;bQG:5";
static const char aGameVersion[] = "1.0";
static const char aCurrency[] = "BXZ";
#else
static const char aGameId[] = "b0c5ba61-fdf4-4744-a0d2-d3e34bc20cd5";
static const char aGameSecret[] = "3R@G@um\\Z!\\xt9{&[A3oOtH}K=oBob`@6xfWGCuE1nR}Gi`gpUyj<G:5";
static const char aGameVersion[] = "1.0";
static const char aCurrency[] = "XWX";
#endif

void xor_secret(char* buf, const char* secret)
{
	register int i;
	const int len = strlen(secret);
	for (i = 0; i < len; ++i)
	{
		buf[i] = secret[i] ^ (i & 15) + 1;
	}
}

// Step 3 Initialize the client-config structure by calling SC_ClientConfig_New()
JNIEXPORT jint JNICALL JNI(SDLActivity_nativeScoreInit)(JNIEnv* env, jobject thisObj)
{
	SC_ClientConfig_h scoreloopClientConfig;
	
//    jclass clazz = (*env)->GetObjectClass(env, thisObj);
//	LOG("nativeScoreInit: clazz=%x obj=%x", clazz, thisObj);
//	
//	jfieldID fid = (*env)->GetStaticFieldID(env, clazz,
//                  "myContext", "Landroid/content/Context;");
//	if (fid == 0)
//	{
//		return -1;
//	}
//	jobject myContext = (*env)->GetStaticObjectField(env, clazz, fid);	

	// Initialize the platform configuration object
	SC_ClientConfig_New(&scoreloopClientConfig);
	// Required: pass a reference to the Android context.
	// When using NativeActivity, the clazz field contains a context.
	SC_ClientConfig_SetAndroidContext(scoreloopClientConfig, thisObj);
	// aGameId, aGameSecret and aCurrency are const char strings that you obtain from Scoreloop.
	// aGameVersion should be your current game version.
	SC_ClientConfig_SetGameIdentifier(scoreloopClientConfig, aGameId);
	char secret_buf[64];
	xor_secret(secret_buf, aGameSecret);
	SC_ClientConfig_SetGameSecret(scoreloopClientConfig, secret_buf);
	SC_ClientConfig_SetGameVersion(scoreloopClientConfig, aGameVersion);
	SC_ClientConfig_SetGameCurrency(scoreloopClientConfig, aCurrency);
	//Step 4
	// Create the client.
	SC_Error_t errCode = SC_Client_NewWithConfig(&scoreloopClient, scoreloopClientConfig);
	LOG("errCode=%d", errCode);
	return errCode;
}

// Step 6 & 7
// Wait for callback - developer's method
void scoreloop_submit_callback(void* userData, SC_Error_t completionStatus)
{
	LOG("scoreloop_submit_callback: userData=%x competionStatus=%d\n", userData, completionStatus);
}

int scoreloop_submit_score(unsigned int scoreMode, unsigned int scoreValue)
{
	SC_Error_t errCode;
	double scoreResult;
	SC_ScoreController_h score_controller;
	SC_Score_h score;
	
	LOG("scoreloop_submit_score: mode=%d score=%d", scoreMode, scoreValue);
	
	//Step 1
	errCode = SC_Client_CreateScore(scoreloopClient, &score);
	//Step 2
	//aResult is the main numerical result achieved by a user in the game.
	SC_Score_SetResult(score, (double) scoreValue);
	//Step 3
	SC_Score_SetMinorResult(score, 0);
	SC_Score_SetMode (score, scoreMode);
	SC_Score_SetLevel (score, 0);	
	//Step 4
	// client - assumes the handle to the client exists
	// aCallback is the callback to be registered
	errCode = SC_Client_CreateScoreController (scoreloopClient, &score_controller, scoreloop_submit_callback, NULL);
	LOG("step4 errCode=%d", errCode);
	//Step 5
	errCode = SC_ScoreController_SubmitScore(score_controller, score);
	LOG("step5 errCode=%d", errCode);
}

SC_ScoresController_h scores_controller;
SC_RankingController_h ranking_controller;
	
void scoreloop_scores_callback(void* userData, SC_Error_t completionStatus)
{
	LOG("scoreloop_scores_callback: userData=%x competionStatus=%d\n", userData, completionStatus);

	SC_ScoreList_h scorelist = SC_ScoresController_GetScores(scores_controller);
	if (scorelist != NULL)
	{
		unsigned int count = SC_ScoreList_GetCount(scorelist);
		unsigned int u;
		
		for (u = 0; u < SCOREBOARD_SIZE; ++u)
		{
			SCORE_ITEM* score = &scoreboard[u];
			if (u < count)
			{
				SC_Score_h score_item = SC_ScoreList_GetAt(scorelist, u);
				SC_User_h user = SC_Score_GetUser(score_item);
				unsigned int hi_score = (unsigned int) SC_Score_GetResult(score_item);
				unsigned int rank = SC_Score_GetRank(score_item);
				SC_String_h login = SC_User_GetLogin(user);
				const char* name = SC_String_GetData(login);
				LOG("rank=%u name=%s score=%u", rank, name, hi_score);
				score->rank = rank;
				score->score = hi_score;
				strcpy(score->name, name);
			}
			else
			{
				score->rank = u + 1;
				memset(score->name, 0, MAX_NAME);
				score->score = 0;
			}
		}
		update_scoreboard_labels();
	}
}

void scoreloop_ranking_callback(void* userData, SC_Error_t completionStatus)
{
	LOG("scoreloop_ranking_callback: userData=%x competionStatus=%d\n", userData, completionStatus);
}

int scoreloop_refresh_scores(unsigned int scoreMode)
{
	unsigned int scoreRange;
	SC_Error_t errCode;
	SC_User_h aUser;
	unsigned int scoreRank;
	SC_ScoreList_h score_list;
	// Step 1
	// client - assumes the handle to the client exists
	// aCallback is the callback to be registered
	// Returns SC_OK is success.
	
	LOG("scoreloop_refresh scores: mode=%d", scoreMode);
	errCode = SC_Client_CreateScoresController(scoreloopClient, &scores_controller, scoreloop_scores_callback, NULL);
//	errCode = SC_Client_CreateRankingController(scoreloopClient, &ranking_controller, scoreloop_ranking_callback, NULL);
	// Step 2
	// setting the search list option
	SC_ScoresController_SetSearchList (scores_controller, SC_SCORES_SEARCH_LIST_ALL);
	// Step 3
	SC_ScoresController_SetMode(scores_controller, scoreMode);
	// Step 4 & 5
	// Request user ranking
	// pass NULL as the user object to load ranking of the session user
//	errCode = SC_RankingController_LoadRankingForUserInMode(ranking_controller, NULL, scoreMode);
//	LOG("step4 errCode=%d", errCode);
	// Request the scores around user
	errCode = SC_ScoresController_LoadScoresAtRank(scores_controller, 1, 10);
	LOG("step5 errCode=%d", errCode);
}

#endif


void init_scoreboard_arrays()
{
    int course, rank;
    list_t course_list=get_score_courses_list();
    list_elem_t cur_elem=get_list_head(course_list);
    if (!arrays_initialized)
    {
		unsigned int u;
		for (u = 0; u < SCOREBOARD_SIZE; ++u)
		{
			SCORE_ITEM* score = &scoreboard[u];
			score->rank = u + 1;
			memset(score->name, 0, MAX_NAME);
			score->score = 0;
		}
        arrays_initialized=True;
    }
}

void refresh_scores_for_course(const char* course_name)
{
#ifdef __ANDROID__
	unsigned int courseID = get_score_index(course_name);
	scoreloop_refresh_scores(courseID);
#else
#endif
}

void init_scoreboard_labels()
{
	unsigned int u;
	char buf[10];
	widget_t* tmp;
    int course;
	for (u=0; u < SCOREBOARD_SIZE; u++)
	{
		sprintf(buf, "%d", u+1);
		rank_labels[u]=create_label(buf);

		name_labels[u]=create_label("");

		score_labels[u]=create_label("");
	}
    init_scoreboard_arrays();
}

void update_scoreboard_labels()
{
	char buf[10];
	int course;
    unsigned int u;
    if (!scoreboard_open)
    {
        return;
    }
    for (u = 0; u < SCOREBOARD_SIZE; u++)
    {
		const SCORE_ITEM* score = &scoreboard[u];
		unsigned int rank = score->rank;
		unsigned int hi_score = score->score;
		sprintf(buf, "%d", rank);
		button_set_text(rank_labels[u], buf);
        if (hi_score)
        {
            button_set_text(name_labels[u], score->name);
			sprintf(buf, "%d", hi_score);
			button_set_text(score_labels[u], buf);
        }
        else
        {
            button_set_text(name_labels[u], "");
			button_set_text(score_labels[u], "");
        }
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

void submit_score(const char* course_name, int course_score)
{
#ifdef __ANDROID__
	unsigned int courseID = get_score_index(course_name);
	scoreloop_submit_score(courseID, course_score);
#else
	print_debug(DEBUG_OTHER, "");
#endif
}
