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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _HUD_H_
#define _HUD_H_

#include "tuxracer.h"

#define OVERSCAN_MARGIN_X                                                      \
  (get_overscan_percent() * getparam_x_resolution() / 100)
#define OVERSCAN_MARGIN_Y                                                      \
  (get_overscan_percent() * getparam_y_resolution() / 100)

void draw_hud(player_data_t *plyr);
void print_fps();

#endif /* _HUD_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
