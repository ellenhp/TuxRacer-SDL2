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

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This code taken from Mesa 3Dfx demos by David Bucciarelli (tech.hmw@plus.it)
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__

SDL_Surface *ImageLoad(const char *filename);

#endif /* !__IMAGE_H__! */

#ifdef __cplusplus
} /* extern "C" */
#endif
