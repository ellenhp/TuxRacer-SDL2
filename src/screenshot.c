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


#include "tuxracer.h"
#include "screenshot.h"

static int screenshot_num = 0;

void screenshot()
{
    char buff[20], *p;
    sprintf( buff, "tux_sshot_%d.ppm", screenshot_num++ );
    p = take_screenshot( buff );
    if ( p != NULL ) {
        fprintf( stderr, "Couldn't save %s: %s\n", buff, p );
    } 
} 

char* take_screenshot ( char* filename ) {
    return (char *)0;
}
