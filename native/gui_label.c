#include <stdlib.h>
#include <string.h>
#include "gui_label.h"

widget_t* create_label(char* text)
{
	widget_t* label=(widget_t*)malloc(sizeof(widget_t));
	label->text=0;
	button_set_text(label, text);
	label->callback1=label->callback2=0;
	label->data1=0;
	label->data2=0;
	label->font_binding=0;
	label->type=LABEL;
	return label;
}
