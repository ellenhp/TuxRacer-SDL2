#ifndef PRIMITIVE_DRAW_H
#define PRIMITIVE_DRAW_H

#include "GLES2/gl2.h"

void draw_textured_quad(float x, float y, float w, float h);
void draw_textured_quad_texcoords(float x, float y, float w, float h,
                                  GLfloat *texCoords);

#endif