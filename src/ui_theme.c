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
#include "ui_theme.h"
#include "textures.h"
#include "shaders.h"
#include "primitive_draw.h"

colour_t ui_background_colour = { 0.48, 0.63, 0.90, 0. };
colour_t ui_foreground_colour = { 1.0, 1.0, 1.0, 1.0 }; 
colour_t ui_highlight_colour = { 1.0, 0.89, 0.01, 1.0 };
colour_t ui_disabled_colour = { 1.0, 1.0, 1.0, 0.6 };
#ifdef TARGET_OS_IPHONE
colour_t ui_enabled_but_disabled_colour = { 1.0, 1.0, 1.0, 1.0 };
#endif

void ui_draw_menu_decorations(bool_t draw_logo)
{
    GLuint texobj;
    char *bl = "menu_bottom_left";
    char *br = "menu_bottom_right";
    char *tl = "menu_top_left";
    char *tr = "menu_top_right";
    char *title = "logo";
    float w = getparam_x_resolution();
    float h = getparam_y_resolution();
    
    float colors[]={1, 1, 1, 1};
    shader_set_color(colors);
    
    glEnable( GL_TEXTURE_2D );
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    shader_set_texture(0);

    /* bottom left */
    if ( get_texture_binding( bl, &texobj ) ) {
        glBindTexture( GL_TEXTURE_2D, texobj );
        draw_textured_quad( 0, 0, h/3, h/3 );
    }


    /* bottom right */
    if ( get_texture_binding( br, &texobj ) ) {
        glBindTexture( GL_TEXTURE_2D, texobj );
        draw_textured_quad( w-h/3, 0, h/3, h/3 );
    }


    /* top left */
    if ( get_texture_binding( tl, &texobj ) ) {
        glBindTexture( GL_TEXTURE_2D, texobj );
        draw_textured_quad( 0, h-h/3, h/3, h/3 );
    }


    /* top right */
    if ( get_texture_binding( tr, &texobj ) ) {
        glBindTexture( GL_TEXTURE_2D, texobj );
        draw_textured_quad( w-h/3, h-h/3, h/3, h/3 );
    }

	if (draw_logo)
	{
		/* title */
		if ( get_texture_binding( title, &texobj ) ) {
			glBindTexture( GL_TEXTURE_2D, texobj );
            draw_textured_quad( w/2-h*0.3, h*0.965-h*0.3, h*0.3*2, h*0.3 );
		}
	}

}
/* EOF */
