#ifndef GUI_JS_INPUT
#define GUI_JS_INPUT

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum input_t
{
	NONE_INPUT=0,
	UP_INPUT=1,
	DOWN_INPUT=1<<1,
	RIGHT_INPUT=1<<2,
	LEFT_INPUT=1<<3,
	SELECT_INPUT=1<<4
} input_t;

void gui_process_keypress(int key);
void gui_process_button_press(int button);
void gui_process_axis(double x, double y);

void gui_input_update();

void gui_set_select_button(int button);

#ifdef __cplusplus
}
#endif

#endif
