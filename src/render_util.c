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
#include "gl_util.h"
#include "render_util.h"
#include "textures.h"
#include "ui_theme.h"

/*
 * Constants 
 */

/* Distance by which to push back far clip plane, to ensure that the
   fogging plane is drawn (m) */
#define FAR_CLIP_FUDGE_AMOUNT 5

static const colour_t text_colour = { 0.0, 0.0, 0.0, 1.0 };

const colour_t white = { 1.0, 1.0, 1.0, 1.0 };
const colour_t grey  = { 0.7, 0.7, 0.7, 1.0 };
const colour_t red   = { 1.0, 0. , 0., 1.0  };
const colour_t green = { 0. , 1.0, 0., 1.0  };
const colour_t blue  = { 0. , 0. , 1.0, 1.0 };
const colour_t light_blue = { 0.5, 0.5, 0.8, 1.0 };
const colour_t black = { 0., 0., 0., 1.0 };
const colour_t sky   = { 0.82, 0.86, 0.88, 1.0 };

const unsigned int PRECISION = 16; 
GLfixed ONE  = 1 << 16 /*PRECISION*/; 
const GLfixed ZERO = 0;

__inline GLfixed FixedFromInt(int value) {return value << PRECISION;}; 
__inline GLfixed FixedFromFloat(float value)  
{ return (GLfixed)value;}; 
__inline GLfixed MultiplyFixed(GLfixed op1, GLfixed op2) 
{ return (op1 * op2) >> PRECISION;};

void glesPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
	GLfloat xmin, xmax, ymin, ymax;
	
	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	
#ifdef HAVE_OPENGLES
	glFrustumf(xmin, xmax, ymin, ymax, zNear, zFar);
#else
	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
#endif
}

void reshape( int w, int h )
{
} 

void flat_mode()
{
}

void draw_overlay() {
    /*
    const GLfloat vertices []=
    {
       0, 0,
       640, 0,
       640, 480,
       0, 480
    };

    glColor4f( 0.0, 0.0, 1.0, 0.1 );
    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (2, GL_FLOAT , 0, vertices);	
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
     */
} 

void clear_rendering_context()
{
    glDepthMask( GL_TRUE );
    glClearColor( ui_background_colour.r,
		  ui_background_colour.g,
		  ui_background_colour.b,
		  ui_background_colour.a );
    glClearStencil( 0 );
    glClear( GL_COLOR_BUFFER_BIT 
	     | GL_DEPTH_BUFFER_BIT 
	     | GL_STENCIL_BUFFER_BIT );
}


void draw_billboard( player_data_t *plyr, 
		     point_t center_pt, scalar_t width, scalar_t height, 
		     bool_t use_world_y_axis, 
		     point2d_t min_tex_coord, point2d_t max_tex_coord )
{
    /*
    point_t pt, pt2, pt3, pt4;
    vector_t x_vec;
    vector_t y_vec;
    vector_t z_vec;

    x_vec.x = plyr->view.inv_view_mat[0][0];
    x_vec.y = plyr->view.inv_view_mat[0][1];
    x_vec.z = plyr->view.inv_view_mat[0][2];

    if ( use_world_y_axis ) {
	y_vec = make_vector( 0, 1, 0 );
	x_vec = project_into_plane( y_vec, x_vec );
	normalize_vector( &x_vec );
	z_vec = cross_product( x_vec, y_vec );
    } else {
	y_vec.x = plyr->view.inv_view_mat[1][0];
	y_vec.y = plyr->view.inv_view_mat[1][1];
	y_vec.z = plyr->view.inv_view_mat[1][2];
	z_vec.x = plyr->view.inv_view_mat[2][0];
	z_vec.y = plyr->view.inv_view_mat[2][1];
	z_vec.z = plyr->view.inv_view_mat[2][2];
    }

    glNormal3f( z_vec.x, z_vec.y, z_vec.z );

    pt = move_point( center_pt, scale_vector( -width/2.0, x_vec ) );
    pt = move_point( pt, scale_vector( -height/2.0, y_vec ) );
    pt2 = move_point( pt, scale_vector( width, x_vec ) );
    pt3 = move_point( pt2, scale_vector( height, y_vec ) );
    pt4 = move_point( pt3, scale_vector( -width, x_vec ) );

	{
    const GLfloat vertices2 []=
    {
       pt.x, pt.y, pt.z,
       pt2.x, pt2.y, pt2.z,
       pt3.x, pt3.y, pt3.z,
       pt4.x, pt4.y, pt4.z
    };

    const GLshort texCoords2 []=
    {
       min_tex_coord.x, min_tex_coord.y,
       max_tex_coord.x, min_tex_coord.y,
       max_tex_coord.x, max_tex_coord.y,
       min_tex_coord.x, max_tex_coord.y,
    };

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);

    glVertexPointer (3, GL_FLOAT , 0, vertices2);	
    glTexCoordPointer(2, GL_SHORT, 0, texCoords2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState (GL_VERTEX_ARRAY);
    glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	}
     */
}
