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
extern "C"
{
#endif

#ifndef _HUD_TRAINING_H_
#define _HUD_TRAINING_H_

#include "tuxracer.h"

void draw_hud_training( player_data_t *plyr);
void init_starting_tutorial_step(int i);
bool_t game_abort_is_for_tutorial(void);
bool_t pause_is_for_long_tutorial_explanation(void);
void training_resume_from_tutorial_explanation(void);

#endif /* _HUD_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
