/*
 * Tux Racer
 * Copyright (C) 1999-2001 Jasmin F. Patry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "joystick.h"
#include "tuxracer.h"

#if defined(HAVE_SDL) && defined(HAVE_SDL_JOYSTICKOPEN)

#include "SDL.h"
#include "SDL_joystick.h"

void update_joystick() {
  // SDL_JoystickUpdate();
}

#else

/* Stub functions */

void init_joystick() {}

bool_t is_joystick_active() { return False; }

void update_joystick() {}

scalar_t get_joystick_x_axis() { return 0.0; }

scalar_t get_joystick_y_axis() { return 0.0; }

bool_t is_joystick_button_down(int button) { return False; }

bool_t is_joystick_continue_button_down() { return False; }

#endif

/* EOF */
