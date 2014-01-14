#include "tuxracer.h"
#include "primitive_draw.h"
#include "winsys.h"
#include "shaders.h"
#include "SDL.h"

void draw_textured_quad(float x, float y, float w, float h)
{
    GLfloat texCoords []=
	{
    	0,0,
    	0,1,
    	1,1,
    	1,0
	};
    draw_textured_quad_texcoords(x, y, w, h, texCoords);
}

void draw_textured_quad_texcoords(float x, float y, float w, float h, GLfloat *texCoords)
{
#define TO_RELATIVE(screen, val) (((val)/screen*2.0)-1.0)
    float screen_w = getparam_x_resolution();
    float screen_h = getparam_y_resolution();
	GLfloat vertices []=
	{
    	TO_RELATIVE(screen_w, x), TO_RELATIVE(screen_h, y), 0,
    	TO_RELATIVE(screen_w, x), TO_RELATIVE(screen_h, y+h), 0,
    	TO_RELATIVE(screen_w, x+w), TO_RELATIVE(screen_h, y+h), 0,
    	TO_RELATIVE(screen_w, x+w), TO_RELATIVE(screen_h, y), 0
	};
	GLubyte indices[]={0, 1, 2, 2, 3, 0};
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 0, texCoords);
    glEnableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));

#undef TO_RELATIVE
}

