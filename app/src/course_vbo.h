#ifndef COURSE_VBO_H
#define COURSE_VBO_H

#include "tuxracer.h"

void init_course_vbo(scalar_t *elevation, terrain_t *terrain, int nx, int nz,
                     scalar_t scalex, scalar_t scalez);

void draw_course_vbo();

#endif
