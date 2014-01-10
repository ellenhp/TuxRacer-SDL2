#ifndef PLATFORM_H
#define PLATFORM_H

#include "tuxracer.h"

int get_overscan_percent();

int get_num_courses();

bool_t is_on_ouya();

char* get_race_text();
char* get_back_text();

char* get_continue_text();
char* get_abort_text();

char* get_select_text();
char* get_quit_text();

#endif