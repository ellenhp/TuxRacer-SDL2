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
#include "render_util.h"
#include "gl_util.h"
#include "hier_util.h"
#include "hier.h"
#include "shaders.h"
#include "alglib.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <deque>

std::deque<glm::mat4> matrix_stack;
glm::mat4 current_mat;

enum firstDraw {
    Yes,
    No
};

struct cas {
    enum firstDraw firstDraw;
    scalar_t* x;
    scalar_t* y;
    scalar_t* z;
};

//-------------------------------------------------------------------------------------------------------------
void PlotSpherePoints(GLfloat radius, GLint stacks, GLint slices, GLfloat* v, GLfloat* n)
{

	GLint i, j; 
	GLfloat slicestep, stackstep;

	stackstep = ((GLfloat)M_PI) / stacks;
	slicestep = 2.0f * ((GLfloat)M_PI) / slices;

	for (i = 0; i < stacks; ++i)		
	{
		GLfloat a = i * stackstep;
		GLfloat b = a + stackstep;

		GLfloat s0 =  (GLfloat)sin(a);
		GLfloat s1 =  (GLfloat)sin(b);

		GLfloat c0 =  (GLfloat)cos(a);
		GLfloat c1 =  (GLfloat)cos(b);

		for (j = 0; j <= slices; ++j)		
		{
			GLfloat c = j * slicestep;
			GLfloat x = (GLfloat)cos(c);
			GLfloat y = (GLfloat)sin(c);

			*n = x * s0;
			*v = *n * radius;

			n++;
			v++;

			*n = y * s0;
			*v = *n * radius;

			n++;
			v++;

			*n = c0;
			*v = *n * radius;

			n++;
			v++;

			*n = x * s1;
			*v = *n * radius;

			n++;
			v++;

			*n = y * s1;
			*v = *n * radius;

			n++;
			v++;

			*n = c1;
			*v = *n * radius;

			n++;
			v++;

		}
	}
}


void
glutSolidSphere(GLfloat radius, GLint slices, GLint stacks) 
{
	GLint i, triangles; 
	static GLfloat* v, *n;
	static GLfloat parms[3];
	if (v) 
	{
		if (parms[0] != radius || parms[1] != slices || parms[2] != stacks) 
		{
			free(v); 
			free(n);

			n = v = 0;
		}
	}

	if (!v) 
	{
		parms[0] = radius; 
		parms[1] = (GLfloat)slices; 
		parms[2] = (GLfloat)stacks;

		v = (GLfloat*)malloc(stacks*(slices+1)*2*3*sizeof *v);
		n = (GLfloat*)malloc(stacks*(slices+1)*2*3*sizeof *n);

     //TRDebugLog("Computing sphere\n");
		PlotSpherePoints(radius, stacks, slices, v, n);
	}

    glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, 0, v);
    glEnableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_NORMAL_NAME), 2, GL_FLOAT, GL_FALSE, 0, n);
    glEnableVertexAttribArray(shader_get_attrib_location(SHADER_NORMAL_NAME));

	triangles = (slices + 1) * 2;

	for(i = 0; i < stacks; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, i * triangles, triangles);

    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_NORMAL_NAME));
}

void draw_sphere( int num_divisions )
{
    int div = num_divisions;
    glutSolidSphere(1, num_divisions+1, num_divisions+1);
}

void hier_push_mat()
{
    matrix_stack.push_back(current_mat);
}

void hier_pop_mat()
{
    if (!matrix_stack.empty())
    {
        current_mat=matrix_stack.back();
        matrix_stack.pop_back();
        util_set_model_matrix(glm::value_ptr(current_mat));
    }
    else
    {
        util_set_translation(0, 0, 0);
    }
}

void hier_mult_mat(glm::mat4 mat)
{
    current_mat=current_mat*mat;
    util_set_model_matrix(glm::value_ptr(current_mat));
}

/*--------------------------------------------------------------------------*/

/* Traverses the DAG structure and draws the nodes
 */
void traverse_dag( scene_node_t *node, material_t *mat )
{
    scene_node_t *child;
    glm::mat4 matrix;
    int i,j;
    
    check_assertion( node != NULL, "node is NULL" );
    hier_push_mat();
    
    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < 4; j++ )
            matrix[i][j] = node->trans[i][j];
    }
    hier_mult_mat(matrix);
    
    if ( node->mat != NULL ) {
        mat = node->mat;
    } 
    
    if ( node->geom == Sphere ) {
        
        //FIXME
        draw_sphere(std::min(MAX_SPHERE_DIVISIONS, std::max(MIN_SPHERE_DIVISIONS, ROUND_TO_NEAREST(getparam_tux_sphere_divisions() * node->param.sphere.divisions))));
    } 
    
    child = node->child;
    while (child != NULL) {
        traverse_dag( child, mat );
        child = child->next;
    } 
    
    hier_pop_mat();
} 

/*--------------------------------------------------------------------------*/

/*
 * make_normal - creates the normal vector for the surface defined by a convex
 * polygon; points in polygon must be specifed in counter-clockwise direction
 * when viewed from outside the shape for the normal to be outward-facing
 */
vector_t make_normal( polygon_t p, point_t *v )
{
    vector_t normal, v1, v2;
    scalar_t old_len;
    
    check_assertion( p.num_vertices > 2, "number of vertices must be > 2" );
    
    v1 = subtract_points( v[p.vertices[1]], v[p.vertices[0]] );
    v2 = subtract_points( v[p.vertices[p.num_vertices-1]], v[p.vertices[0]] );
    normal = cross_product( v1, v2 );
    
    old_len = normalize_vector( &normal );
    check_assertion( old_len > 0, "normal vector has length 0" );
    
    return normal;
} 

/*--------------------------------------------------------------------------*/

/* Returns True iff the specified polygon intersections a unit-radius sphere
 * centered at the origin.  */
bool_t intersect_polygon( polygon_t p, point_t *v )
{
    ray_t ray; 
    vector_t nml, edge_nml, edge_vec;
    point_t pt;
    double d, s, nuDotProd, wec;
    double edge_len, t, distsq;
    int i;
    
    /* create a ray from origin along polygon normal */
    nml = make_normal( p, v );
    ray.pt = make_point( 0., 0., 0. );
    ray.vec = nml;
    
    nuDotProd = dot_product( nml, ray.vec );
    if ( fabs(nuDotProd) < EPS )
        return False;
    
    /* determine distance of plane from origin */
    d = -( nml.x * v[p.vertices[0]].x + 
          nml.y * v[p.vertices[0]].y + 
          nml.z * v[p.vertices[0]].z );
    
    /* if plane's distance to origin > 1, immediately reject */
    if ( fabs( d ) > 1 )
        return False;
    
    /* check distances of edges from origin */
    for ( i=0; i < p.num_vertices; i++ ) {
        point_t *v0, *v1;
        
        v0 = &v[p.vertices[i]];
        v1 = &v[p.vertices[ (i+1) % p.num_vertices ]]; 
        
        edge_vec = subtract_points( *v1, *v0 );
        edge_len = normalize_vector( &edge_vec );
        
        /* t is the distance from v0 of the closest point on the line
         to the origin */
        t = - dot_product( *((vector_t *) v0), edge_vec );
        
        if ( t < 0 ) {
            /* use distance from v0 */
            distsq = MAG_SQD( *v0 );
        } else if ( t > edge_len ) {
            /* use distance from v1 */
            distsq = MAG_SQD( *v1 );
        } else {
            /* closest point to origin is on the line segment */
            *v0 = move_point( *v0, scale_vector( t, edge_vec ) );
            distsq = MAG_SQD( *v0 );
        }
        
        if ( distsq <= 1 ) {
            return True;
        }
    }
    
    /* find intersection point of ray and plane */
    s = - ( d + dot_product( nml, make_vector(ray.pt.x, ray.pt.y, ray.pt.z) ) ) /
    nuDotProd;
    
    pt = move_point( ray.pt, scale_vector( s, ray.vec ) );
    
    /* test if intersection point is in polygon by clipping against it 
     * (we are assuming that polygon is convex) */
    for ( i=0; i < p.num_vertices; i++ ) {
        edge_nml = cross_product( nml, 
                                 subtract_points( v[p.vertices[ (i+1) % p.num_vertices ]], v[p.vertices[i]] ) );
        
        wec = dot_product( subtract_points( pt, v[p.vertices[i]] ), edge_nml );
        if (wec < 0)
            return False;
    } 
    
    return True;
} 

/*--------------------------------------------------------------------------*/

/* returns True iff polyhedron intersects unit-radius sphere centered
 at origin */
bool_t intersect_polyhedron( polyhedron_t p )
{
    bool_t hit = False;
    int i;
    
    for (i=0; i<p.num_polygons; i++) {
        hit = intersect_polygon( p.polygons[i], p.vertices );
        if ( hit == True ) 
            break;
    } 
    return hit;
} 

/*--------------------------------------------------------------------------*/

polyhedron_t copy_polyhedron( polyhedron_t ph )
{
    int i;
    polyhedron_t newph = ph;
    newph.vertices = (point_t *) malloc( sizeof(point_t) * ph.num_vertices );
    for (i=0; i<ph.num_vertices; i++) {
        newph.vertices[i] = ph.vertices[i];
    } 
    return newph;
} 

/*--------------------------------------------------------------------------*/

void free_polyhedron( polyhedron_t ph ) 
{
    free(ph.vertices);
} 

/*--------------------------------------------------------------------------*/

void trans_polyhedron( matrixgl_t mat, polyhedron_t ph )
{
    int i;
    for (i=0; i<ph.num_vertices; i++) {
        ph.vertices[i] = transform_point( mat, ph.vertices[i] );
    } 
} 

/*--------------------------------------------------------------------------*/

bool_t check_polyhedron_collision_with_dag( 
                                           scene_node_t *node, matrixgl_t modelMatrix, matrixgl_t invModelMatrix,
                                           polyhedron_t ph)
{
    matrixgl_t newModelMatrix, newInvModelMatrix;
    scene_node_t *child;
    polyhedron_t newph;
    bool_t hit = False;
    
    check_assertion( node != NULL, "node is NULL" );
    
    multiply_matrices( newModelMatrix, modelMatrix, node->trans );
    multiply_matrices( newInvModelMatrix, node->invtrans, invModelMatrix );
    
    if ( node->geom == Sphere ) {
        newph = copy_polyhedron( ph );
        trans_polyhedron( newInvModelMatrix, newph );
        
        hit = intersect_polyhedron( newph );
        
        free_polyhedron( newph );
    } 
    
    if (hit == True) return hit;
    
    child = node->child;
    while (child != NULL) {
        
        hit = check_polyhedron_collision_with_dag( 
                                                  child, newModelMatrix, newInvModelMatrix, ph );
        
        if ( hit == True ) {
            return hit;
        }
        
        child = child->next;
    } 
    
    return False;
} 

