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

/* This code taken from Mesa 3Dfx demos by David Bucciarelli (tech.hmw@plus.it)
 */

#include "tuxracer.h"
#include "image.h"
#include "SDL_image.h"

void flipSurface(SDL_Surface* surface)
{
	char* tmp;
	int i;
	int pitch=surface->pitch;
	tmp=(char*)malloc(surface->pitch);
	SDL_LockSurface(surface);
	for (i=0; i<surface->h/2; i++)
	{
		memcpy(tmp, (char*)surface->pixels+i*pitch, pitch);
		memcpy((char*)surface->pixels+i*pitch, (char*)surface->pixels+pitch*(surface->h-1)-i*pitch, pitch);
		memcpy((char*)surface->pixels+pitch*(surface->h-1)-i*pitch, tmp, pitch);
	}
	SDL_UnlockSurface(surface);
	free(tmp);
}

SDL_Surface* ImageLoad(const char *filename)
{
	SDL_Surface *image = IMG_Load(filename);
	if (!image) {
		handle_error(1, "Error loading image: %s", filename);
	}
	print_debug(DEBUG_OTHER, "%d, %d", image->flags, image->format->format);
	flipSurface(image);
	return image;
}
