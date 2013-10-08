#include "ui_mgr.h"
#include "gui_mgr.h"
#include "gui_input.h"
#include "fonts.h"
#include "gui_abstraction.h"
#include "winsys.h"
#include "joystick.h"

font_t* menu_font;

/*
 * This is here to allow ui_mgr to know about the position of the cursor so it can render it on top of the menus
 */
void GameMenu_motion_func( int x, int y )
{
	ui_event_motion_func( x, y );
	gui_mouse_motion_event( x, y );
}

/*
 * This is here solely to hide the internals of the GUI library from tux racer
 */
void GameMenu_mouse_func( int button, int state, int x, int y )
{
	gui_mouse_button_event( button, state, x, y );
}

void GameMenu_joystick_func( double x, double y )
{
	gui_process_axis(x, y);
}

void GameMenu_joystick_button_func( int button )
{
	gui_process_button_press(button);
}

void GameMenu_keypress(int key)
{
	gui_process_keypress(key);
}

void GameMenu_simulate_click(widget_t* widget)
{
	//call back is the same regardless of what the widget type is
	widget_bounding_box_t bb={0, 0, 0, 0};
	widget->callback1(1, 0, 0, bb, CLICK_INPUT);
}

void GameMenu_init()
{

}

void GameMenu_draw()
{
	gui_update();
	gui_draw();
}

coord_t get_absolute(coord_t coord, int asc, int desc)
{
	int w, h, num_lines;
	coord_t absolute;

	w=GameMenu_get_window_width();
	h=GameMenu_get_window_height();

	absolute.x_coord_type=ABSOLUTE_COORD;
	absolute.y_coord_type=ABSOLUTE_COORD;
	absolute.x_just=coord.x_just;
	absolute.y_just=coord.y_just;

	switch (coord.x_coord_type)
	{
	case ABSOLUTE_COORD:
		absolute.x=coord.x;
		break;
	case NORMALIZED_COORD:
		absolute.x=(int)((coord.x+1)/2.0f*w);
		break;
	case LINE_COORD:
		print_warning( IMPORTANT_WARNING,"LINE coords only available on Y axis.\n" );
		break;
	}

	switch (coord.y_coord_type)
	{
	case ABSOLUTE_COORD:
		absolute.y=coord.y;
		break;
	case NORMALIZED_COORD:
		absolute.y=(int)((coord.y+1)/2.0f*h);
		break;
	case LINE_COORD:
		absolute.y_just=CENTER_JUST; //justification is overriden in lines mode
		num_lines=(int)((float)h/asc+desc);
		if (num_lines%2==0)
			num_lines--; // num_lines should be odd so we have a line 0 centered vertically
		absolute.y=h/2-coord.y*(asc+desc);
		break;
	}
	return absolute;
}

widget_bounding_box_t GameMenu_get_bb(widget_t* widget)
{
	widget_bounding_box_t bb;

    font_t *font;
    int w, asc, desc;
	coord_t absolute_coord;

    if (!get_font_binding( "button_label", &font ))
	{
        print_warning( IMPORTANT_WARNING, 
                        "Couldn't get font object for binding %s",
                        "button_label" );
        font = NULL;
    }
	else
	{
		bind_font_texture( font );
		get_font_metrics( font, widget->text, &w, &asc, &desc );
	}
	
	absolute_coord=get_absolute(widget->layout_info, asc, desc);

	bb.x=absolute_coord.x;
	bb.y=GameMenu_get_window_height()-absolute_coord.y;
	bb.width=w;
	bb.height=asc+desc;

	switch (absolute_coord.x_just)
	{
	case RIGHT_JUST:
		bb.x-=bb.width;
		break;
	case CENTER_JUST:
		bb.x-=bb.width/2;
		break;
	default:
        print_warning( IMPORTANT_WARNING,"Justification x/y mismatch\n" );
		break;
	}
	
	switch (absolute_coord.y_just)
	{
	case BOTTOM_JUST:
		bb.y-=bb.height;
		break;
	case CENTER_JUST:
		bb.y-=bb.height/2;
		break;
	default:
        print_warning( IMPORTANT_WARNING,"Justification x/y mismatch\n" );
		break;
	}
	return bb;
}

void GameMenu_draw_text(char* text, int active, coord_t coord)
{
    font_t *font;
    int w, asc, desc;
	coord_t absolute_coord;
	int x_render_pos, y_render_pos;

    if (!get_font_binding( active ? "button_label_hilit" : "button_label" , &font )) {
        print_warning( IMPORTANT_WARNING, 
                        "Couldn't get font object for binding %s",
                        "button_label" );
        font = NULL;
    }
	else
	{
		bind_font_texture( font );
		get_font_metrics( font, text, &w, &asc, &desc );


		absolute_coord=get_absolute(coord, asc, desc);

		switch (coord.x_just)
		{
		case LEFT_JUST:
			x_render_pos=absolute_coord.x;
			break;
		case RIGHT_JUST:
			x_render_pos=absolute_coord.x-w;
			break;
		case CENTER_JUST:
			x_render_pos=absolute_coord.x-w/2;
			break;
		default:
	        print_warning( IMPORTANT_WARNING,"Justification x/y mismatch\n" );
			return;
			break;
		}

		switch (coord.y_just)
		{
		case BOTTOM_JUST:
			y_render_pos=absolute_coord.y;
			break;
		case TOP_JUST:
			y_render_pos=absolute_coord.y+asc+desc;
			break;
		case CENTER_JUST:
			y_render_pos=absolute_coord.y-asc/2.0+desc/2.0;
			break;
		default:
	        print_warning( IMPORTANT_WARNING,"Justification x/y mismatch\n" );
			return;
			break;
		}
		
        glPushMatrix();
        {
			glTranslatef( x_render_pos, y_render_pos, 0.0 );

            draw_string( font, text );
        }
        glPopMatrix();
    }
}

int GameMenu_get_window_height()
{
	return getparam_y_resolution();
}

int GameMenu_get_window_width()
{
	return getparam_x_resolution();
}

int GameMenu_resolve_bounds(int val, int min, int max, input_type_t input_type)
{
	if (input_type==CLICK_INPUT)
	{
		if (val>max)
		{
			val=val%(max+1)+min;
		}
		if (val<max)
		{
			val%=max+1;
			val+=max+1;
			val%=max+1;
			val+=min;
		}
	}
	else //joystick or keyboard directional input
	{
		if (val>max)
		{
			val=max;
		}
		if (val<min)
		{
			val=min;
		}
	}
	return val;
}

