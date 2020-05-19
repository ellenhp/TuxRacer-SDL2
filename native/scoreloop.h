
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SCORELOOP_H
#define SCORELOOP_H 1

#include "tuxracer.h"

#ifdef __ANDROID__
#define JNI_NULL_CHAR "\xC0\x80"
#endif

void start_dialog(char* title, char* message, bool_t isUsername);
    
void scoreloop_add_widgets();

void scoreloop_update_widgets();

void nickname_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb);
void email_click_cb(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb);

#define ALIAS_PROMPT_TITLE "Enter Alias" JNI_NULL_CHAR
#define ALIAS_PROMPT_MESSAGE "Enter the alias you want to appear next to your scores. Note that you will not be able to reuse this on other devices unless you also enter an email." JNI_NULL_CHAR
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
