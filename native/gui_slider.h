#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "gui_mgr.h"

widget_t* create_slider(char* text, int num_options, char* options, widget_click_callback_t changed);

void slider_value_changed(int button, int mouse_x, int mouse_y, widget_bounding_box_t bb, input_type_t input_type, widget_t* widget);

#ifdef __cplusplus
}
#endif

#endif
