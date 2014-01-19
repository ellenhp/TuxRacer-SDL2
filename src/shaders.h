#ifndef SHADERS_H
#define SHADERS_H

#ifdef __cplusplus
extern "C"
{
#endif
    
#include "tuxracer.h"

#define SHADER_VERTEX_NAME "vert_pos"
#define SHADER_TEXTURE_COORD_NAME "source_tex_coord"
#define SHADER_NORMAL_NAME "source_normal"
#define SHADER_TERRAINS_NAME "source_terrain"
#define SHADER_TERRAIN_TEXTURES_NAME "terrain"
#define SHADER_ENVMAP_NAME "envmap"
    
#define SHADER_LIGHT_POSITION_NAME "lightpos"
#define SHADER_LIGHT_SPECULAR_NAME "lightspecular"
#define SHADER_LIGHT_DIFFUSE_NAME "lightdiffuse"
#define SHADER_LIGHT_AMBIENT_NAME "lightambient"

void init_programs();
void use_terrain_program();
void use_generic_program();
void use_hud_program();
void use_tux_program();

void shader_set_texture(GLuint texture);
void shader_set_color(GLfloat* argb);

GLuint shader_get_attrib_location(char* name);
GLuint shader_get_uniform_location(char* name);
    
#ifdef __cplusplus
}
#endif
    
#endif