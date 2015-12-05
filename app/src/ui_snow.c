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
#include "ui_snow.h"
#include "gl_util.h"
#include "course_render.h"
#include "render_util.h"
#include "game_config.h"
#include "textures.h"
#include "ui_mgr.h"
#include "loop.h"
#include "primitive_draw.h"
#include "shaders.h"

#define MAX_NUM_PARTICLES 10000
#define PARTICLE_DENSITY 0.0005
#define GRAVITY_FACTOR 0.015
#define BASE_VELOCITY 0.05
#define VELOCITY_RANGE 0.02
#define PUSH_DECAY_TIME_CONSTANT 0.2
#define PUSH_DIST_DECAY 100 
#define PUSH_FACTOR 0.5
#define MAX_PUSH_FORCE 5
#define WIND_FORCE 0.03
#define AIR_DRAG 0.4

#define PARTICLE_MIN_SIZE 1
#define PARTICLE_SIZE_RANGE 10



typedef struct _particle_t {
    point2d_t pt;
    scalar_t size;
    vector2d_t vel;
    point2d_t tex_min;
    point2d_t tex_max;
} particle_t;

static particle_t particles[MAX_NUM_PARTICLES];
static int orig_num_particles = 0;
static int num_particles = 0;
static GLfloat particle_colour[4] = { 1, 1, 1, 0.4 };
static point2d_t push_position = {0, 0};
static point2d_t last_push_position;
static scalar_t last_update_time = -1;
static bool_t push_position_initialized = False;

static scalar_t frand() 
{
    return (scalar_t)rand()/RAND_MAX;
} 

static void make_particle( int i, scalar_t x, scalar_t y )
{
    scalar_t p_dist;
    int type;

    particles[i].pt.x = x;
    particles[i].pt.y = y;
    p_dist = frand();
    particles[i].size = PARTICLE_MIN_SIZE + (1.0 - p_dist)*PARTICLE_SIZE_RANGE;
    particles[i].vel.x = 0;
    particles[i].vel.y = -BASE_VELOCITY-p_dist*VELOCITY_RANGE;
    type = (int) (frand() * (4.0 - EPS));
    if (type == 0) {
	particles[i].tex_min = make_point2d( 0.0, 0.0 );
	particles[i].tex_max = make_point2d( 0.5, 0.5 );
    } else if (type == 1) {
	particles[i].tex_min = make_point2d( 0.5, 0.0 );
	particles[i].tex_max = make_point2d( 1.0, 0.5 );
    } else if (type == 2) {
	particles[i].tex_min = make_point2d( 0.5, 0.5 );
	particles[i].tex_max = make_point2d( 1.0, 1.0 );
    } else {
	particles[i].tex_min = make_point2d( 0.0, 0.5 );
	particles[i].tex_max = make_point2d( 0.5, 1.0 );
    }
}

void init_ui_snow( void )
{
    int i;

	num_particles=orig_num_particles=PARTICLE_DENSITY*getparam_x_resolution()*getparam_y_resolution();

    for( i=0; i<num_particles; i++) {
	make_particle( i, frand(), frand() );
    }
    push_position = make_point2d( 0.0, 0.0 );
}

void update_ui_snow( scalar_t time_step, bool_t windy )
{
    vector2d_t *v, f;
    point2d_t *pt;
    scalar_t size;
    scalar_t dist_from_push, p_dist;
    vector2d_t push_vector;
    int i;
    scalar_t push_timestep, time;
    float grav_x, grav_y;

    time = get_clock_time();

    push_vector.x = 0;
    push_vector.y = 0;
    push_timestep = 0;
#ifdef TARGET_OS_IPHONE_
    winsys_get_gravity(&grav_x, &grav_y);
#else
	// No gravity (yet) on Windows/Android
	grav_x = 0.0;
	grav_y = -1.0;
#endif
    if ( push_position_initialized ) {
	push_vector.x = push_position.x - last_push_position.x;
	push_vector.y = push_position.y - last_push_position.y;
	push_timestep = time - last_update_time;
    }
    last_push_position = push_position;
    last_update_time = time;

    for ( i=0; i<num_particles; i++) {
	pt = &particles[i].pt;
	v = &particles[i].vel;
	size = particles[i].size;

	f.x = 0;
	f.y = 0;

	/* Mouse push and gravity */
	dist_from_push = (pow((pt->x - push_position.x), 2) +
			  pow((pt->y - push_position.y), 2));
	if ( push_timestep > 0 ) {
	    f.x = PUSH_FACTOR * push_vector.x / push_timestep; 
	    
	    f.y = PUSH_FACTOR * push_vector.y / push_timestep; 

	    f.x = min( MAX_PUSH_FORCE, f.x );
	    f.x = max( -MAX_PUSH_FORCE, f.x );
	    f.y = min( MAX_PUSH_FORCE, f.y );
	    f.y = max( -MAX_PUSH_FORCE, f.y );

	    f.x *= 1.0/(PUSH_DIST_DECAY*dist_from_push + 1) * 
		size/PARTICLE_SIZE_RANGE;
	    f.y *= 1.0/(PUSH_DIST_DECAY*dist_from_push + 1) *
		size/PARTICLE_SIZE_RANGE;
	}

	/* Update velocity */
	v->x += ( f.x + GRAVITY_FACTOR * grav_x + ( windy ? WIND_FORCE : 0.0 ) - v->x * AIR_DRAG ) * 
	    time_step;
	v->y += ( f.y + GRAVITY_FACTOR * grav_y - v->y * AIR_DRAG ) * 
	    time_step;

	/* Update position */
        pt->x += v->x * time_step * ( size / PARTICLE_SIZE_RANGE ); 
        pt->y += v->y * time_step * ( size / PARTICLE_SIZE_RANGE );

	if ( pt->x < 0 ) {
	    pt->x = 1;
	} else if ( pt->x > 1 ) {
	    pt->x = 0.0;
	}
    }

    /* Kill off & regenerate particles */
    for (i=0; i<num_particles; i++) {
	particle_t *p = &particles[i];

	if (p->pt.y < -0.05) {
	    /* If we have an excess of particles, kill off with
	       50% probability */
	    if ( num_particles > orig_num_particles && frand() > 0.5 ) {
		/* Delete the particle */
		*p = particles[num_particles-1];
		num_particles -= 1;
	    } else {
		p->pt.x = frand();
		p->pt.y = 1+frand()*BASE_VELOCITY;
		p_dist = frand();
		p->size = PARTICLE_MIN_SIZE + 
		    ( 1.0 - p_dist ) * PARTICLE_SIZE_RANGE;
		p->vel.x = 0;
		p->vel.y = -BASE_VELOCITY-p_dist*VELOCITY_RANGE;
	    }
	}
    }

    if ( time_step < PUSH_DECAY_TIME_CONSTANT ) {
	push_vector.x *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
	push_vector.y *= 1.0 - time_step/PUSH_DECAY_TIME_CONSTANT;
    } else {
	push_vector.x = 0.0;
	push_vector.y = 0.0;
    }
}

void init_arrays_for_particle(particle_t particle, GLfloat* vertices, GLfloat* tex_coords)
{
    point2d_t pt = particle.pt;
    scalar_t size = particle.size;
    point2d_t* tex_min = &particle.tex_min;
    point2d_t* tex_max = &particle.tex_max;
    scalar_t xres, yres;

#define TO_RELATIVE(screen, val) (((val)/screen*2.0)-1.0)

    xres=getparam_x_resolution();
    yres=getparam_y_resolution();
    
    vertices[0]=TO_RELATIVE(xres, pt.x*xres);
    vertices[1]=TO_RELATIVE(yres, pt.y*yres);
    
    vertices[2]=TO_RELATIVE(xres, pt.x*xres);
    vertices[3]=TO_RELATIVE(yres, pt.y*yres+size);
    
    vertices[4]=TO_RELATIVE(xres, pt.x*xres+size);
    vertices[5]=TO_RELATIVE(yres, pt.y*yres+size);
    
    vertices[6]=TO_RELATIVE(xres, pt.x*xres+size);
    vertices[7]=TO_RELATIVE(yres, pt.y*yres);
    
#undef TO_RELATIVE
    
    tex_coords[0]=tex_min->x;
    tex_coords[1]=tex_min->y;
    
    tex_coords[2]=tex_min->x;
    tex_coords[3]=tex_max->y;
    
    tex_coords[4]=tex_max->x;
    tex_coords[5]=tex_max->y;
    
    tex_coords[6]=tex_max->x;
    tex_coords[7]=tex_min->y;
}

void draw_ui_snow( void )
{
    GLuint   texture_id;
    char *binding;
    point2d_t *pt, *tex_min, *tex_max;
    scalar_t size;
    scalar_t xres, yres;
    int i;

    xres = getparam_x_resolution();
    yres = getparam_y_resolution();
    
    ui_setup_display();

    binding = "snow_particle";
    if (!get_texture_binding( "snow_particle", &texture_id ) ) {
	print_warning( IMPORTANT_WARNING,
		       "Couldn't get texture for binding %s", 
		       binding );
	texture_id = 0;
    }

    glBindTexture( GL_TEXTURE_2D, texture_id );
    
    shader_set_color(particle_colour);
    
    {
        GLfloat* vertices=(GLfloat*)malloc(8*num_particles*sizeof(GLfloat));
        GLfloat* tex_coords=(GLfloat*)malloc(8*num_particles*sizeof(GLfloat));
        GLushort* indices=(GLushort*)malloc(6*num_particles*sizeof(GLushort));
        
        for ( i=0; i<num_particles; i++) {
            int firstvert=i*4;
            int firstindex=i*6;
            init_arrays_for_particle(particles[i], vertices+i*8, tex_coords+i*8);
            indices[0+firstindex]=0+firstvert;
            indices[1+firstindex]=1+firstvert;
            indices[2+firstindex]=2+firstvert;
            indices[3+firstindex]=2+firstvert;
            indices[4+firstindex]=3+firstvert;
            indices[5+firstindex]=0+firstvert;

        }
        
        //print_debug(DEBUG_OTHER, "indices %d %d %d %d %d %d %d %d %d %d %d %d", indices[0], indices[1], indices[2], indices[3], indices[4], indices[5], indices[6], indices[7], indices[8], indices[9], indices[10], indices[11]);
        //print_debug(DEBUG_OTHER, "vertices %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", vertices[0], vertices[1], vertices[2], vertices[3], vertices[4], vertices[5], vertices[6], vertices[7], vertices[8], vertices[9], vertices[10], vertices[11], vertices[12], vertices[13], vertices[14], vertices[15]);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
        
        glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 0, tex_coords);
        glEnableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));
        
        glDrawElements(GL_TRIANGLES, num_particles*6, GL_UNSIGNED_SHORT, indices);
        
        glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDisableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
        
        glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDisableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));

        free(vertices);
        free(tex_coords);
    }
} 

void
reset_ui_snow_cursor_pos( point2d_t pos ) 
{
    scalar_t xres, yres;

    xres = getparam_x_resolution();
    yres = getparam_y_resolution();
    push_position = make_point2d( pos.x/(scalar_t)xres,
				  pos.y/(scalar_t)yres );
    last_push_position = push_position;
    push_position_initialized = True;
}

void 
push_ui_snow( point2d_t pos )
{
    scalar_t xres, yres;

	return;

    xres = getparam_x_resolution();
    yres = getparam_y_resolution();
    push_position = make_point2d( pos.x/(scalar_t)xres,
				  pos.y/(scalar_t)yres );
    if ( !push_position_initialized ) {
	last_push_position = push_position;
    }
    push_position_initialized = True;
}

void
make_ui_snow( point2d_t pos ) {
    scalar_t xres, yres;

    xres = getparam_x_resolution();
    yres = getparam_y_resolution();

    if ( num_particles < MAX_NUM_PARTICLES ) {
	make_particle( num_particles, pos.x/xres, pos.y/yres );
	num_particles++;
    }
}


void
reset_ui_snow( void ) {
    scalar_t xres, yres;
	int i;
    
    xres = getparam_x_resolution();
    yres = getparam_y_resolution();
    
    /* Kill off & regenerate particles */
    for (i=0; i<num_particles; i++) {
        particle_t *p = &particles[i];
        
                /* Delete the particle */
                *p = particles[num_particles-1];
                num_particles -= 1;
    }
    
    num_particles = orig_num_particles;
    
    init_ui_snow();
}
