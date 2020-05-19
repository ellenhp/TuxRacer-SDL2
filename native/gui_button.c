#include <stdlib.h>
#include <string.h>
#include "gui_button.h"

widget_t* create_button(char* text, widget_click_callback_t click_cb)
{
	widget_t* btn=(widget_t*)malloc(sizeof(widget_t));
	btn->text=0;
	button_set_text(btn, text);
	btn->callback1=click_cb;
	btn->data1=0;
	btn->data2=0;
	btn->font_binding=0;
    btn->active=0;
	btn->type=BUTTON;
	return btn;
}
