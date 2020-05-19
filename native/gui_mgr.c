#include <stdlib.h>
#include "gui_mgr.h"
#include "gui_abstraction.h"
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL.h>

widget_list_item_t* gamemenu_top;

widget_list_item_t* active_item;

int mouse_x, mouse_y;

void gui_mouse_motion_event(int x, int y)
{
	mouse_x=x;
	mouse_y=y;
}

void gui_mouse_button_event(int button, int down, int x, int y)
{
	widget_click_callback_t cb=0;
    if (down)
	{
		widget_list_item_t* current=gamemenu_top;
		while (current)
		{
			widget_bounding_box_t bb;
            if (current==active_item)
            {
                current->widget->active=1;
                bb=GameMenu_get_bb(current->widget);
                current->widget->active=0;
            }
            else
            {
                bb=GameMenu_get_bb(current->widget);
            }
			if (x>bb.x && x<bb.x+bb.width &&
				y>bb.y && y<bb.y+bb.height &&
				current->widget->type!=LABEL)
			{
				if (active_item==current)
                {
                    if (button==SDL_BUTTON_LEFT) //left mouse button
                    {
                        cb=active_item->widget->callback1; //up callback
                    }
                    if (cb)
                    {
                        cb(button, x, y, bb, CLICK_INPUT, active_item->widget);
                    }
                }
                else
                {
                    active_item=current;
                }
                return;
			}
			current=current->next;
		}
		if (!current) //end of the loop was reached
		{
			active_item=NULL;
            return;
		}
	}
	if (active_item)
	{
		widget_bounding_box_t bb=GameMenu_get_bb(active_item->widget);
		if (x>bb.x && x<bb.x+bb.width &&
			y>bb.y && y<bb.y+bb.height)
		{
			switch (active_item->widget->type)
			{
                case BUTTON:
                    if (active_item->widget->callback1)
                    {
                        active_item->widget->callback1(button, x, y, bb, CLICK_INPUT, active_item->widget);
                    }
                    active_item=NULL;
                    break;
			}
		}
	}
}

#define LEFT_ARROW	"\x05"
#define RIGHT_ARROW	"\x06"

void gui_draw()
{
	widget_list_item_t* current=gamemenu_top;
	char text[100];
	while (current)
	{
		switch (current->widget->type)
		{
		case SLIDER:
			if (active_item==current)
			{
				sprintf(text, LEFT_ARROW " %s " RIGHT_ARROW, current->widget->text);
			}
			else
			{
				sprintf(text, "%s", current->widget->text);
			}
			break;
		default:
			sprintf(text, "%s", current->widget->text);
			break;
		}
		GameMenu_draw_text(text, active_item==current, current->widget->layout_info, current->widget->font_binding);
		
		current=current->next;
	}


}

void gui_update()
{
	gui_input_update();
}

void gui_add_widget(widget_t* widget, coord_t* coord)
{
	//current functionality appends item to the end of the linked list
	widget_list_item_t* item;
	widget_list_item_t* current=gamemenu_top;
    widget_list_item_t* new_active_item;

	int last_line=INT_MIN;
	int next_line=0;

	if (current)
	{
		widget_list_item_t* last;
		while (current)
		{
			if (!coord && //if appending
				current->widget->layout_info.y_coord_type == LINE_COORD && //and this widget's y coord is in lines
				last_line < current->widget->layout_info.y) //and it's line coord is bigger than the biggest one so far
			{
				last_line=current->widget->layout_info.y; //update last_line
				next_line=last_line+1; //and update next_line
			}
			last=current;
			current=current->next;
		}
		current=last;

		item=(widget_list_item_t*)malloc(sizeof(widget_list_item_t));
		current->next=item;
		item->prev=current;
		item->next=0;
		item->widget=widget;
	}
	else
	{
		//the list is empty
		gamemenu_top=current=(widget_list_item_t*)malloc(sizeof(widget_list_item_t));
		current->prev=0;
		current->next=0;
		current->widget=widget;
	}
	
	if (!coord)
	{
		widget->layout_info.x=0.5;
		widget->layout_info.x_coord_type=NORMALIZED_COORD;
		widget->layout_info.x_just=CENTER_JUST;
		widget->layout_info.y=next_line;
		widget->layout_info.y_coord_type=LINE_COORD;
		widget->layout_info.y_just=CENTER_JUST;
	}
	else
	{
		widget->layout_info=*coord;
	}

	if (active_item == NULL)
	{
		new_active_item=gamemenu_top;
        while (new_active_item->widget->type==LABEL)
        {
            new_active_item=new_active_item->next;
            if (new_active_item==NULL)
            {
                return;
            }
        }
        active_item=new_active_item;
	}
}

void gui_process_input(input_t input)
{
	widget_bounding_box_t bb={0, 0, 0, 0};
	widget_list_item_t* new_active_item=active_item;
	switch (input)
	{
	case NONE_INPUT:
		return;
	case SELECT_INPUT:
		if (active_item!=NULL)
		{
			active_item->widget->callback1(1, 0, 0, bb, CLICK_INPUT, active_item->widget);
		}
		break;
	default:
		//directional input
		if (active_item==NULL)
		{
			new_active_item=gamemenu_top;
			while (new_active_item && new_active_item->widget->type==LABEL)
			{
				new_active_item=new_active_item->next;
			}
			active_item=new_active_item;
			return;
		}
		//another switch. I don't think it's possible to combine them without duplicating the active_item==NULL check
		switch (input)
		{
		case UP_INPUT:
			do
			{
				new_active_item=new_active_item->prev;
			} while  (new_active_item && new_active_item->widget->type==LABEL);
			if (new_active_item)
				active_item=new_active_item;
			break;
		case DOWN_INPUT:
			do
			{
				new_active_item=new_active_item->next;
			} while (new_active_item && new_active_item->widget->type==LABEL);
			if (new_active_item)
				active_item=new_active_item;
			return;
			break;
		case RIGHT_INPUT:
			if (active_item->widget->type==SLIDER && active_item->widget->callback1)
				active_item->widget->callback1(SDL_BUTTON_RIGHT, 0, 0, bb, JOY_KB_INPUT, active_item->widget);
			break;
		case LEFT_INPUT:
			if (active_item->widget->type==SLIDER && active_item->widget->callback1)
				active_item->widget->callback1(SDL_BUTTON_LEFT, 0, 0, bb, JOY_KB_INPUT, active_item->widget);
			break;
		}
	}
}

void gui_balance_lines(int manual_offset)
{
	widget_list_item_t* current=gamemenu_top;
	int min_line=INT_MAX, max_line=INT_MIN;
	int offset;
	while (current)
	{
		if (current->widget->layout_info.y_coord_type==LINE_COORD)
		{
			const int y = current->widget->layout_info.y;
			if (min_line > y)
			{
				min_line = y;
			}
			if (max_line < y)
			{
				max_line = y;
			}
		}
		current=current->next;
	}
	offset=min_line-(max_line-min_line)/2 + manual_offset;

	current=gamemenu_top;
	while (current)
	{
		if (current->widget->layout_info.y_coord_type==LINE_COORD)
		{
			current->widget->layout_info.y+=offset;
		}
		current=current->next;
	}

}

void button_set_text(widget_t* widget, char* text)
{
	char* text_cpy=(char*)malloc(strlen(text)+1);
	if (widget->text)
	{
		free(widget->text);
		widget->text=0;
	}
	strcpy(text_cpy, text);
	widget->text=text_cpy;
}

void slider_set_value(widget_t* slider, int value)
{
	char* label=0;
	int i=0;
	char* next_delimiter=slider->data2;
	char* substring_end=0;
	int substring_length=0;
	char* format;

	char tmp[100];

	slider->option=value;
	
	if (slider->text)
	{
		free(slider->text);
		slider->text=0;
	}

	if (slider->data2)
	{
		for (i=0; i<value; i++)
		{
			next_delimiter=strchr(next_delimiter, SLIDER_DELIMITER);
			next_delimiter++; //move to the next character after the delimiter
			if (next_delimiter==0)
			{
				button_set_text(slider, "ERROR");
				return;
			}
		}

		substring_end=strchr(next_delimiter, SLIDER_DELIMITER);

		if (substring_end)
		{
			substring_length=substring_end-next_delimiter;
		}
		else
		{
			substring_length=strlen(next_delimiter);
		}

		slider->text=(char*)malloc(strlen(slider->data1) + substring_length + 1+4);

		memcpy(tmp, next_delimiter, substring_length);
		tmp[substring_length]=0; //null terminate the string

		sprintf(slider->text, "%s%s", slider->data1, tmp);
	}
	else
	{
		slider->text=(char*)malloc(strlen(slider->data1)+10);

		sprintf(slider->text, "%s%d", slider->data1, slider->option);
	}
}

void destroy_widget(widget_t* widget)
{
	if (widget==0)
	{
		return;
	}
	if (widget->data1)
	{
		free(widget->data1);
	}
	if (widget->data2)
	{
		free(widget->data2);
	}
	if (widget->data2)
	{
		free(widget->text);
	}
	free(widget);
}

void reset_gui()
{
	widget_list_item_t* current=gamemenu_top;
	widget_list_item_t* tmp;
	while (current)
	{
		destroy_widget(current->widget);
		current->widget=0;
		tmp=current;
		current=current->next;
		free(tmp);
	}
	active_item=NULL;
	gamemenu_top=NULL;
}

void setup_gui()
{
	reset_gui();
}