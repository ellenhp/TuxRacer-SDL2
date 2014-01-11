#ifndef PLATFORM_H
#define PLATFORM_H

#include "tuxracer.h"

/* OUYA Font "characters" */
#define OUYA_O_BUTTON	"\x01 "
#define OUYA_U_BUTTON	"\x02 "
#define OUYA_Y_BUTTON	"\x03 "
#define OUYA_A_BUTTON	"\x04 "

int get_overscan_percent(void);

int get_num_courses(void);

bool_t is_on_ouya(void);

char* get_race_text(void);
char* get_back_text(void);

char* get_continue_text(void);
char* get_abort_text(void);

char* get_select_text(void);
char* get_quit_text(void);

#endif