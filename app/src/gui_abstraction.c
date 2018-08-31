
#include "gui_abstraction.h"
#include "fonts.h"
#include "gui_input.h"
#include "gui_mgr.h"
#include "joystick.h"
#include "ui_mgr.h"
#include "winsys.h"

font_t *menu_font;

double y_offset = 0;

/*
 * This is here to allow ui_mgr to know about the position of the cursor so it
 * can render it on top of the menus
 */
void GameMenu_motion_func(int x, int y) {
  ui_event_motion_func(x, y);
  gui_mouse_motion_event(x, y);
}

/*
 * This is here solely to hide the internals of the GUI library from tux racer
 */
void GameMenu_mouse_func(int button, int state, int finger_index, int x,
                         int y) {
  gui_mouse_button_event(button, state, x, y);
}

void GameMenu_joystick_func(double x, double y) {
  if (winsys_is_controller_active()) {
    gui_process_axis(x, y);
  }
}

void GameMenu_joystick_button_func(int button) {
  gui_process_button_press(button);
}

void GameMenu_keypress(int key) { gui_process_keypress(key); }

void GameMenu_simulate_click(widget_t *widget) {
  // call back is the same regardless of what the widget type is
  widget_bounding_box_t bb = {0, 0, 0, 0};
  widget->callback1(1, 0, 0, bb, CLICK_INPUT, widget);
}

void GameMenu_init() {
  y_offset = 0;
  gui_set_select_button(getparam_joystick_continue_button());
}

void GameMenu_draw() {
  gui_update();
  gui_draw();
}

void GameMenu_set_y_offset(double offset) { y_offset = offset; }

coord_t get_absolute(coord_t coord, int asc, int desc) {
  double w, h;
  int num_lines;
  coord_t absolute;

  w = GameMenu_get_window_width();
  h = GameMenu_get_window_height();

  absolute.x_coord_type = ABSOLUTE_COORD;
  absolute.y_coord_type = ABSOLUTE_COORD;
  absolute.x_just = coord.x_just;
  absolute.y_just = coord.y_just;

  switch (coord.x_coord_type) {
  case ABSOLUTE_COORD:
    absolute.x = (int)coord.x;
    break;
  case NORMALIZED_COORD:
    absolute.x = (int)(coord.x * w);
    break;
  case LINE_COORD:
    print_warning(IMPORTANT_WARNING, "LINE coords only available on Y axis.\n");
    break;
  }

  switch (coord.y_coord_type) {
  case ABSOLUTE_COORD:
    absolute.y = (int)coord.y;
    break;
  case NORMALIZED_COORD:
    absolute.y = (int)(coord.y * h);
    break;
  case LINE_COORD:
    absolute.y_just = CENTER_JUST; // justification is overriden in lines mode
    num_lines = (int)(h / asc + desc);
    if (num_lines % 2 == 0)
      num_lines--; // num_lines should be odd so we have a line 0 centered
                   // vertically
    absolute.y = (int)(h / 2 - coord.y * (asc + desc));
    break;
  }
  absolute.y += y_offset * h;
  return absolute;
}

#define LEFT_ARROW "\x05"
#define RIGHT_ARROW "\x06"

widget_bounding_box_t GameMenu_get_bb(widget_t *widget) {
  widget_bounding_box_t bb;
  float extra_size = 0.3;

  font_t *font;
  int w, asc, desc;
  coord_t absolute_coord;
  char text[100];

  if (!get_font_binding("button_label", &font)) {
    print_warning(IMPORTANT_WARNING, "Couldn't get font object for binding %s",
                  "button_label");
    font = NULL;
  } else {
    bind_font_texture(font);
    if (widget->active) {
      sprintf(text, LEFT_ARROW " %s " RIGHT_ARROW, widget->text);
    } else {
      strcpy(text, widget->text);
    }
    get_font_metrics(font, text, &w, &asc, &desc);
  }

  absolute_coord = get_absolute(widget->layout_info, asc, desc);

  bb.x = absolute_coord.x;
  bb.y = GameMenu_get_window_height() - absolute_coord.y -
         extra_size * (asc + desc);
  bb.width = w;
  bb.height = (1 + extra_size) * (asc + desc);

  switch (absolute_coord.x_just) {
  case RIGHT_JUST:
    bb.x -= bb.width;
    break;
  case CENTER_JUST:
    bb.x -= bb.width / 2;
    break;
  case LEFT_JUST:
    // do nothing
    break;
  default:
    print_warning(IMPORTANT_WARNING, "Justification x/y mismatch 0\n");
    break;
  }

  switch (absolute_coord.y_just) {
  case BOTTOM_JUST:
    bb.y -= bb.height;
    break;
  case CENTER_JUST:
    bb.y -= bb.height / 2;
    break;
  case TOP_JUST:
    // do nothing
    break;
  default:
    print_warning(IMPORTANT_WARNING, "Justification x/y mismatch 1\n");
    break;
  }
  return bb;
}

void GameMenu_draw_text(const char *text, int active, coord_t coord,
                        char *requested_font_binding) {
  font_t *font;
  char *font_binding;
  int w, asc, desc;
  coord_t absolute_coord;
  double x_render_pos, y_render_pos;

  if (requested_font_binding == 0) {
    font_binding = active ? "button_label_hilit" : "button_label";
  } else {
    font_binding = requested_font_binding;
  }

  if (!get_font_binding(font_binding, &font)) {
    print_warning(IMPORTANT_WARNING, "Couldn't get font object for binding %s",
                  "button_label");
    font = NULL;
  } else {
    bind_font_texture(font);
    get_font_metrics(font, text, &w, &asc, &desc);

    absolute_coord = get_absolute(coord, asc, desc);

    switch (coord.x_just) {
    case LEFT_JUST:
      x_render_pos = absolute_coord.x;
      break;
    case RIGHT_JUST:
      x_render_pos = absolute_coord.x - w;
      break;
    case CENTER_JUST:
      x_render_pos = absolute_coord.x - w / 2;
      break;
    default:
      print_warning(IMPORTANT_WARNING, "Justification x/y mismatch 2\n");
      return;
      break;
    }

    switch (coord.y_just) {
    case BOTTOM_JUST:
      y_render_pos = absolute_coord.y;
      break;
    case TOP_JUST:
      y_render_pos = absolute_coord.y + asc + desc;
      break;
    case CENTER_JUST:
      y_render_pos = absolute_coord.y - asc / 2.0 + desc / 2.0;
      break;
    default:
      print_warning(IMPORTANT_WARNING, "Justification x/y mismatch 3\n");
      return;
      break;
    }

    draw_string(font, text, x_render_pos, y_render_pos);
  }
}

void rect_to_absolute(rect_t *rect) {
  if (rect->lower_left.x_coord_type == NORMALIZED_COORD) {
    rect->lower_left.x_coord_type = ABSOLUTE_COORD;
    rect->lower_left.x *= GameMenu_get_window_width();
  }
  if (rect->lower_left.y_coord_type == NORMALIZED_COORD) {
    rect->lower_left.y_coord_type = ABSOLUTE_COORD;
    rect->lower_left.y *= GameMenu_get_window_height();
  }
  if (rect->upper_right.x_coord_type == NORMALIZED_COORD) {
    rect->upper_right.x_coord_type = ABSOLUTE_COORD;
    rect->upper_right.x *= GameMenu_get_window_width();
  }
  if (rect->upper_right.y_coord_type == NORMALIZED_COORD) {
    rect->upper_right.y_coord_type = ABSOLUTE_COORD;
    rect->upper_right.y *= GameMenu_get_window_height();
  }
}

void GameMenu_draw_image(GLuint binding, rect_t image_rect,
                         rect_t screen_rect) {
  GLfloat texCoords[] = {image_rect.upper_right.x, image_rect.lower_left.y,
                         image_rect.upper_right.x, image_rect.upper_right.y,
                         image_rect.lower_left.x,  image_rect.upper_right.y,
                         image_rect.lower_left.x,  image_rect.lower_left.y};
  GLubyte indices[] = {0, 1, 2, 0, 2, 3};

  glBindTexture(GL_TEXTURE_2D, binding);

  rect_to_absolute(&screen_rect);

  draw_textured_quad_texcoords(
      screen_rect.lower_left.x, screen_rect.lower_left.y,
      screen_rect.upper_right.x - screen_rect.lower_left.x,
      screen_rect.upper_right.y - screen_rect.lower_left.y, texCoords);
}

void GameMenu_draw_image_full(GLuint binding, rect_t screen_rect) {
  glBindTexture(GL_TEXTURE_2D, binding);

  rect_to_absolute(&screen_rect);

  draw_textured_quad(screen_rect.lower_left.x, screen_rect.lower_left.y,
                     screen_rect.upper_right.x - screen_rect.lower_left.x,
                     screen_rect.upper_right.y - screen_rect.lower_left.y);
}

int GameMenu_get_window_height() { return getparam_y_resolution(); }

int GameMenu_get_window_width() { return getparam_x_resolution(); }

int GameMenu_resolve_bounds(int val, int min, int max,
                            input_type_t input_type) {
  if (input_type == CLICK_INPUT) {
    if (val > max) {
      val = val % (max + 1) + min;
    }
    if (val < min) {
      val %= max + 1;
      val += max + 1;
      val %= max + 1;
      val += min;
    }
  } else // joystick or keyboard directional input
  {
    if (val > max) {
      val = max;
    }
    if (val < min) {
      val = min;
    }
  }
  return val;
}
