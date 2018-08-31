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

#include "tux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _HIER_H_
#define _HIER_H_

#define MIN_SPHERE_DIVISIONS 3
#define MAX_SPHERE_DIVISIONS 16

extern int get_scene_node(const char *node_name, scene_node_t **node);

extern const char *reset_scene_node(const char *node);
extern const char *rotate_scene_node(const char *node, char axis,
                                     scalar_t angle);
extern const char *translate_scene_node(const char *node, vector_t trans);
extern const char *scale_scene_node(const char *node, point_t origin,
                                    scalar_t factor[3]);
extern const char *transform_scene_node(const char *node, matrixgl_t mat,
                                        matrixgl_t invMat);

extern const char *set_scene_node_material(const char *node, const char *mat);
extern const char *create_material(const char *mat, colour_t d, colour_t s,
                                   scalar_t s_exp);

extern const char *set_scene_resolution(const char *resolution);

extern const char *set_scene_node_shadow_state(const char *node,
                                               const char *state);
extern const char *set_scene_node_eye(const char *node, const char *which_eye);

extern const char *create_tranform_node(const char *parent, const char *name);
extern const char *create_sphere_node(const char *parent_name,
                                      const char *child_name,
                                      scalar_t resolution);

extern void initialize_scene_graph();

extern void draw_scene_graph(const char *node);
extern bool_t collide(const char *node, polyhedron_t ph);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
