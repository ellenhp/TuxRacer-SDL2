#ifndef SHADERS_H
#define SHADERS_H

#ifdef __cplusplus
extern "C"
{
#endif
    
#include "tuxracer.h"

#define SHADER_VERTEX_NAME "vert_pos"
#define SHADER_TEXTURE_COORD_NAME "source_tex_coord"

void init_programs();
void use_terrain_program();
void use_generic_program();

void shader_set_texture(GLuint texture);
void shader_set_color(GLfloat* argb);

GLuint shader_get_attrib_location(char* name);
GLuint shader_get_uniform_location(char* name);
    
#ifdef __cplusplus
}
#endif
    
#endif