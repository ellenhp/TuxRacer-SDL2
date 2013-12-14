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

colour_t ui_background_colour = { 0.48, 0.63, 0.90, 0. };
colour_t ui_foreground_colour = { 1.0, 1.0, 1.0, 1.0 }; 
colour_t ui_highlight_colour = { 1.0, 0.89, 0.01, 1.0 };
colour_t ui_disabled_colour = { 1.0, 1.0, 1.0, 0.6 };
#ifdef TARGET_OS_IPHONE
colour_t ui_enabled_but_disabled_colour = { 1.0, 1.0, 1.0, 1.0 };
#endif

static void draw_quad(int x, int y, int w, int h)
{
	GLfloat vertices []=
	{
    	0, 0, 0,
    	0, h, 0,
    	w, h, 0,
    	w, 0, 0
	};
    GLfloat texCoords []=
	{
    	0,0,
    	0,1,
    	1,1,
    	1,0
	};
	GLubyte indices[]={0, 1, 2, 0, 2, 3};

	glPushMatrix();
    {
		glTranslatef( x, y, 0 );
    	
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_FLOAT , 0, vertices);	
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    glPopMatrix();
}

void ui_draw_menu_decorations(bool_t draw_logo)
{
    GLuint texobj;
    char *bl = "menu_bottom_left";
    char *br = "menu_bottom_right";
    char *tl = "menu_top_left";
    char *tr = "menu_top_right";
    char *title = "logo";
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();

    glEnable( GL_TEXTURE_2D );
    glColor4f( 1., 1., 1., 1. );

    /* bottom left */
    if ( get_texture_binding( bl, &texobj ) ) {
        glBindTexture( GL_TEXTURE_2D, texobj );
        draw_quad( 0, 0, h/3, h/3 );
    }


    /* bottom right */
    if ( get_texture_binding( br, &texobj ) ) {
        glBindTexture( GL_TEXTURE_2D, texobj );
        draw_quad( w-h/3, 0, h/3, h/3 );
    }


    /* top left */
    if ( get_texture_binding( tl, &texobj ) ) {
        glBindTexture( GL_TEXTURE_2D, texobj );
        draw_quad( 0, h-h/3, h/3, h/3 );
    }


    /* top right */
    if ( get_texture_binding( tr, &texobj ) ) {
        glBindTexture( GL_TEXTURE_2D, texobj );
        draw_quad( w-h/3, h-h/3, h/3, h/3 );
    }

	if (draw_logo)
	{
		/* title */
		if ( get_texture_binding( title, &texobj ) ) {
			glBindTexture( GL_TEXTURE_2D, texobj );
			draw_quad( w/2-h/3, h*0.95-h/3, h/3*2, h/3 );
		}
	}

}
/* EOF */
