#ifndef SHADERS_H
#define SHADERS_H

#include "tuxracer.h"

#define SHADER_VERTEX_NAME "vert_pos"
#define SHADER_TEXTURE_COORD_NAME "source_tex_coord"

void init_programs();
void load_terrain_program();
void load_generic_program();

void shader_set_texture(GLuint texture);
GLuint shader_get_attrib_location(char* name);

#endif