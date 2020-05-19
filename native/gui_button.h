#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "gui_mgr.h"

#ifdef __cplusplus
extern "C"
{
#endif

widget_t* create_button(char* text, widget_click_callback_t click_cb);

#ifdef __cplusplus
}
#endif

#endif
