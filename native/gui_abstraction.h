#ifndef GUI_ABSTRACTION_H
#define GUI_ABSTRACTION_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#include "windows.h"
#include "GL/GL.h"
#else
#include "GLES2/gl2.h"
#endif
#include "gui_mgr.h"

    void GameMenu_motion_func(int x, int y);
    void GameMenu_mouse_func(int button, int state, int finger_index, int x, int y);
    void GameMenu_joystick_func(double x, double y);
    void GameMenu_joystick_button_func(int button);

    void GameMenu_keypress(int key);

    void GameMenu_simulate_click(widget_t *widget);

    void GameMenu_set_y_offset(double offset);

    void GameMenu_init();
    void GameMenu_draw();

    widget_bounding_box_t GameMenu_get_bb(widget_t *widget);
    void GameMenu_draw_text(const char *text, int active, coord_t coord, char *requested_font_binding);
    void GameMenu_draw_image(GLuint binding, rect_t image_rect, rect_t screen_rect);
    void GameMenu_draw_image_full(GLuint binding, rect_t screen_rect);

    coord_t get_absolute(coord_t coord, int asc, int desc);

    int GameMenu_get_window_height();
    int GameMenu_get_window_width();

    int GameMenu_resolve_bounds(int val, int min, int max, input_type_t input_type);

#ifdef __cplusplus
}
#endif

#endif