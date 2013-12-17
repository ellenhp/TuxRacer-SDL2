#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "gui_mgr.h"
#include "tuxracer.h"

#define RANK_USER 0
#define SCOREBOARD_SIZE 10
#define MAX_COURSES 50

extern bool_t scoreboard_open;

void refresh_scores();
void refresh_scores_for_course(char* course);

void init_scoreboard_labels();
void update_scoreboard_labels();

/*
 * Pass RANK_USER to request user's labels
 * Return NULL if RANK_USER PASSED and the user is in the top 10
 */
widget_t* get_name_label(int rank);
widget_t* get_score_label(int rank);
widget_t* get_rank_label(int rank);

void insert_score(char* course_name, char* name, int score);

#endif