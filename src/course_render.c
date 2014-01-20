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
#include "course_load.h"
#include "course_render.h"
#include "textures.h"
#include "phys_sim.h"
#include "hier_util.h"
#include "gl_util.h"
#include "render_util.h"
#include "track_marks.h"
#include "shaders.h"
#ifdef TARGET_OS_IPHONE
    #include "sharedGeneralFunctions.h"
    #include <sys/mman.h>
    #include <fcntl.h>
#endif
#include "multiplayer.h"


/* 
 *  Constants 
 */

/* How long to make the flat part at the bottom of the course, as a
 fraction of the total length of the course */
#define FLAT_SEGMENT_FRACTION 0.2

/* Aspect ratio of background texture */
#define BACKGROUND_TEXTURE_ASPECT 3.0


/*
 * Statics 
 */

/* The course normal vectors */
static vector_t *nmls = NULL;

/* Should we activate clipping when drawing the course? */
static bool_t clip_course = False;

/* If clipping is active, it will be based on a camera located here */
static point_t eye_pt;

static tree_t* treeLocsOrderedByZ=NULL;

// VVV NNN TT
// aligned on 32-byte intervals
static GLuint trees_vbo=0;

/* Macros for converting indices in height map to world coordinates */
#define XCD(x) (  (scalar_t)(x) / (nx-1.) * courseWidth )
#define ZCD(y) ( -(scalar_t)(y) / (ny-1.) * courseLength )

#define NORMAL(x, y) ( nmls[ (x) + nx * (y) ] )


/*
 * Function definitions
 */

void set_course_clipping( bool_t state ) { clip_course = state; }
void set_course_eye_point( point_t pt ) { eye_pt = pt; }

vector_t* get_course_normals() { return nmls; } 

#ifdef TARGET_OS_IPHONE

static size_t nmls_len = 0;
static int nmls_fd = -1;

#endif

void calc_normals(const char *course)
{
    scalar_t *elevation;
    scalar_t courseWidth, courseLength;
    int nx, ny;
    int x,y;
    point_t p0, p1, p2;
    vector_t n, nml, v1, v2;
    char buff[BUFF_LEN];
    struct stat buf;
    int exists;

    //sprintf( buff, "%s/courses/%s/normal.data", getparam_data_dir(), course );

    get_course_dimensions( &courseWidth, &courseLength );
    get_course_divisions( &nx, &ny );
    elevation = get_course_elev_data();
    get_course_dimensions( &courseWidth, &courseLength );
    get_course_divisions( &nx, &ny );

    if ( nmls != NULL ) {
        free( nmls );
    } 

    nmls = (vector_t *)malloc( sizeof(vector_t)*nx*ny ); 
    if ( nmls == NULL ) {
	handle_system_error( 1, "malloc failed" );
    }

                
        for ( y=0; y<ny; y++) {
            for ( x=0; x<nx; x++) {
                nml = make_vector( 0., 0., 0. );
                
                p0 = make_point( XCD(x), ELEV(x,y), ZCD(y) );
                
                /* The terrain is meshed as follows:
                 ...
                 +-+-+-+-+            x<---+
                 |\|/|\|/|                 |
                 ...+-+-+-+-+...              V
                 |/|\|/|\|                 y
                 +-+-+-+-+
                 ...
                 
                 So there are two types of vertices: those surrounded by
                 four triangles (x+y is odd), and those surrounded by
                 eight (x+y is even).
                 */
                
    #define POINT(x,y) make_point( XCD(x), ELEV(x,y), ZCD(y) )
                
                if ( (x + y) % 2 == 0 ) {
                    if ( x > 0 && y > 0 ) {
                        p1 = POINT(x,  y-1);
                        p2 = POINT(x-1,y-1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                        
                        p1 = POINT(x-1,y-1);
                        p2 = POINT(x-1,y  );
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                    } 
                    if ( x > 0 && y < ny-1 ) {
                        p1 = POINT(x-1,y  );
                        p2 = POINT(x-1,y+1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                        
                        p1 = POINT(x-1,y+1);
                        p2 = POINT(x  ,y+1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                    } 
                    if ( x < nx-1 && y > 0 ) {
                        p1 = POINT(x+1,y  );
                        p2 = POINT(x+1,y-1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                        
                        p1 = POINT(x+1,y-1);
                        p2 = POINT(x  ,y-1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                    } 
                    if ( x < nx-1 && y < ny-1 ) {
                        p1 = POINT(x+1,y  );
                        p2 = POINT(x+1,y+1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v1, v2 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                        
                        p1 = POINT(x+1,y+1);
                        p2 = POINT(x  ,y+1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v1, v2 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                        
                    } 
                } else {
                    /* x + y is odd */
                    if ( x > 0 && y > 0 ) {
                        p1 = POINT(x,  y-1);
                        p2 = POINT(x-1,y  );
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                    } 
                    if ( x > 0 && y < ny-1 ) {
                        p1 = POINT(x-1,y  );
                        p2 = POINT(x  ,y+1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                    } 
                    if ( x < nx-1 && y > 0 ) {
                        p1 = POINT(x+1,y  );
                        p2 = POINT(x  ,y-1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v2, v1 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                    } 
                    if ( x < nx-1 && y < ny-1 ) {
                        p1 = POINT(x+1,y  );
                        p2 = POINT(x  ,y+1);
                        v1 = subtract_points( p1, p0 );
                        v2 = subtract_points( p2, p0 );
                        n = cross_product( v1, v2 );
                        
                        check_assertion( n.y > 0, "course normal points down" );
                        
                        normalize_vector( &n );
                        nml = add_vectors( nml, n );
                    } 
                }
                
                normalize_vector( &nml );
                NORMAL(x,y) = nml;
                continue;
            } 
    #undef POINT
        }
#if TARGET_IPHONE_SIMULATOR
        munmap(nmls, nmls_len);
        close(nmls_fd);

        nmls_fd = open(buff, O_RDONLY);
        if (nmls_fd == -1) {
            handle_system_error( 1, "can't remount normal.data" );
        }

        TRDebugLog("remounting to memory normal.data\n");
        nmls_len = sizeof(vector_t)*nx*ny;
        nmls = mmap(NULL, nmls_len, PROT_READ, MAP_SHARED, nmls_fd, 0);
        if ( nmls == (void *)-1 ) {
            handle_system_error( 1, "remount mmap failed" );
        }
#endif
#undef NORMAL
}

#define DRAW_POINT \
glNormal3f( nml.x, nml.y, nml.z ); \
glVertex3f( pt.x, pt.y, pt.z ); 

void render_course()
{
    set_gl_options( COURSE );
    
    draw_course_vbo();
    
    draw_track_marks();
}

void draw_sky(point_t pos)
{
    GLuint texture_id[6];
    
#define DIST (getparam_forward_clip_distance()*0.7)
    
    GLfloat vertices []=
    {
        -DIST, -DIST, -DIST,
        DIST, -DIST, -DIST,
        DIST,  DIST, -DIST,
        -DIST,  DIST, -DIST,
        -DIST, -DIST, -DIST,
        DIST,  DIST, -DIST,
        
        -DIST,  DIST, -DIST,
        DIST,  DIST, -DIST,
        DIST,  DIST,  DIST,
        -DIST,  DIST,  DIST,
        -DIST,  DIST, -DIST,
        DIST,  DIST,  DIST,
        
        -DIST, -DIST,  DIST,
        DIST, -DIST,  DIST,
        DIST, -DIST, -DIST,
        -DIST, -DIST, -DIST,
        -DIST, -DIST,  DIST,
        DIST, -DIST, -DIST,
        
        -DIST, -DIST,  DIST,
        -DIST, -DIST, -DIST,
        -DIST,  DIST, -DIST,
        -DIST,  DIST,  DIST,
        -DIST, -DIST,  DIST,
        -DIST,  DIST, -DIST,
        
        DIST, -DIST, -DIST,
        DIST, -DIST,  DIST,
        DIST,  DIST,  DIST,
        DIST,  DIST, -DIST,
        DIST, -DIST, -DIST,
        DIST,  DIST,  DIST,
        
        DIST, -DIST,  DIST,
        -DIST, -DIST,  DIST,
        -DIST,  DIST,  DIST,
        DIST,  DIST,  DIST,
        DIST, -DIST,  DIST,
        -DIST,  DIST,  DIST,
    };
    
    GLfloat texCoords []=
    {
        // Work around an iphone FPU/GL? bug
        // that makes the plane and texture non contiguous.
        // this removes an artifacts visible in the sky

		// nopoe 10/13/2013: necessary even on windows when using OpenGL ES rendering code

#define ZERO 0.005
#define ONE 0.995
        ZERO, ZERO ,
        ONE, ZERO ,
        ONE, ONE ,
        ZERO, ONE ,
        ZERO, ZERO ,
        ONE, ONE ,
        
        ZERO, ZERO ,
        ONE, ZERO ,
        ONE, ONE ,
        ZERO, ONE ,
        ZERO, ZERO ,
        ONE, ONE ,
        
        ZERO, ZERO ,
        ONE, ZERO ,
        ONE, ONE ,
        ZERO, ONE ,
        ZERO, ZERO ,
        ONE, ONE ,
        
        ZERO, ZERO ,
        ONE, ZERO ,
        ONE, ONE ,
        ZERO, ONE ,
        ZERO, ZERO ,
        ONE, ONE ,
        
        ZERO, ZERO ,
        ONE, ZERO ,
        ONE, ONE ,
        ZERO, ONE ,
        ZERO, ZERO ,
        ONE, ONE ,
        
        ZERO, ZERO ,
        ONE, ZERO ,
        ONE, ONE ,
        ZERO, ONE ,
        ZERO, ZERO ,
        ONE, ONE ,
    };

    int i = 0;
    
    float white[]={1, 1, 1, 1};

	set_gl_options( SKY );
    
    if (!(get_texture_binding( "sky_front", &texture_id[0] ) && 
          get_texture_binding( "sky_top", &texture_id[1] ) && 
          get_texture_binding( "sky_bottom", &texture_id[2] ) && 
          get_texture_binding( "sky_left", &texture_id[3] ) && 
          get_texture_binding( "sky_right", &texture_id[4] ) && 
          get_texture_binding( "sky_back", &texture_id[5] ) ) ) {
        return;
    }
    
    util_set_translation(pos.x, pos.y, pos.z);
    shader_set_color(white);
        
    glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 0, texCoords);
    glEnableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));

    for(i =0; i < 6; i++) {
        glBindTexture( GL_TEXTURE_2D, texture_id[i] );
        
        glDrawArrays(GL_TRIANGLES, i*6, 6);
        
    }
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));

    util_set_translation(0, 0, 0);
}

/* fonction utilisateur de comparaison fournie a qsort() */
static int compare_func (void const *a, void const *b)
{
    tree_t const *treeA = (tree_t*)a;
    tree_t const *treeB = (tree_t*)b;
	scalar_t diff;
    
    point_t locA,locB;
    int ret = 0;
    
    locA = (*treeA).ray.pt;
    locB = (*treeB).ray.pt;
    
    diff = fabs(locA.z) - fabs(locB.z);
    
    if (diff > 0)
    {
        ret = 1;
    }
    else if (diff < 0)
    {
        ret = -1;
    }
    
    return ret;
}

int order_trees_by_z (tree_t* treeLocs,int num_trees)
{
    qsort (treeLocs, num_trees, sizeof(tree_t), compare_func);
    return 0;
}

void draw_trees() 
{
    float white[]={1, 1, 1, 1};
    GLuint texobj, item_type;
    int numItems, vertex, i;
    item_t* itemLocs;
    char* item_name;
    GLfloat itemRadius, itemHeight;
    vector_t normal;
    item_type_t* item_types;
    
    int current_item=0;
    
    const GLfloat verticesTemplateItem[]=
    {
        -1.0, 0.0,  1.0,
        1.0, 0.0, -1.0,
        1.0, 1.0, -1.0,
        -1.0, 1.0,  1.0,
        -1.0, 0.0,  1.0,
        1.0, 1.0, -1.0,
    };
    
    const GLfloat texCoordsTemplateItem[]=
    {
        0.0, 0.0,
        1.0/4, 0.0,
        1.0/4, 1.0,
        0.0, 1.0,
        0.0, 0.0,
        1.0/4, 1.0,
    };
    GLfloat* verticesItem;
    GLfloat* texCoordsItem;
    
    shader_set_color(white);
    
    if (!get_texture_binding("trees", &texobj) ) {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, texobj);

    set_gl_options(TREES);
    
    glBindBuffer(GL_ARRAY_BUFFER, trees_vbo);
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, 32, (GLvoid*)0);
    glEnableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 32, (GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));
    
    glDrawArrays(GL_TRIANGLES, 0, 12*get_num_trees());
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //working on items now
    
    itemLocs = get_item_locs();
    numItems = get_num_items();
    
    verticesItem=(GLfloat*)malloc(numItems*18*sizeof(GLfloat));
    texCoordsItem=(GLfloat*)malloc(numItems*12*sizeof(GLfloat));
    
    item_types = get_item_types();
    
    for (i = 0; i< numItems; i++) {
        GLfloat xOffset, yOffset, zOffset;
        if (!game_has_herring()) {
            if ( itemLocs[i].collectable == 1) {
                continue;
            }
        }
        if ( itemLocs[i].collectable == 0 || itemLocs[i].drawable == False) {
            continue;
        }
        
        if (itemLocs[i].item_type != item_type) {
            item_type = itemLocs[i].item_type;
            item_name = get_item_name(item_type);
            if (!get_texture_binding(item_name, &texobj)) {
                texobj = 0;
            }
            glBindTexture(GL_TEXTURE_2D, texobj);
        }
        
        xOffset=itemLocs[i].ray.pt.x;
        yOffset=itemLocs[i].ray.pt.y;
        zOffset=itemLocs[i].ray.pt.z;
        
        itemRadius = itemLocs[i].diam/2.;
        itemHeight = itemLocs[i].height;
        
        if (item_types[item_type].use_normal ) {
            normal = item_types[item_type].normal;
        } else {
            normal = subtract_points( eye_pt, itemLocs[i].ray.pt );
            normalize_vector( &normal );
        }
        
        if (normal.y == 1.0) {
            continue;
        }
        
        normal.y = 0.0;
        normalize_vector( &normal );
        
        for (vertex=0; vertex<6; vertex++)
        {
            verticesItem[current_item*18+0+vertex*3]=xOffset + itemRadius * normal.z * verticesTemplateItem[0+vertex*3];
            verticesItem[current_item*18+1+vertex*3]=yOffset + itemHeight * verticesTemplateItem[1+vertex*3];
            verticesItem[current_item*18+2+vertex*3]=zOffset + itemRadius * normal.x * verticesTemplateItem[2+vertex*3];
            
            texCoordsItem[current_item*12+0+vertex*2]=texCoordsTemplateItem[0+vertex*2]+item_types[item_type].atlas_index/4.0;
            texCoordsItem[current_item*12+1+vertex*2]=texCoordsTemplateItem[1+vertex*2];
        }
        
        current_item++;
    }
    
    if (!get_texture_binding("items", &texobj) ) {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, texobj);

    glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, 0, verticesItem);
    glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 0, texCoordsItem);
    
    glDrawArrays(GL_TRIANGLES, 0, current_item*6);
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    
    glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));
    
    free(verticesItem);
    free(texCoordsItem);
}

/*! 
 Draws a fog plane at the far clipping plane to mask out clipping of terrain.
 
 \return  none
 \author  jfpatry
 \date    Created:  2000-08-31
 \date    Modified: 2000-08-31
 */
void draw_fog_plane()
{
    /*
    plane_t left_edge_plane, right_edge_plane;
    plane_t left_clip_plane, right_clip_plane;
    plane_t far_clip_plane;
    plane_t bottom_clip_plane;
    plane_t bottom_plane, top_plane;
    
    scalar_t course_width, course_length;
    scalar_t course_angle, slope;
    
    point_t left_pt, right_pt, pt;
    point_t top_left_pt, top_right_pt;
    point_t bottom_left_pt, bottom_right_pt;
    vector_t left_vec, right_vec;
    scalar_t height;
    
    GLfloat *fog_colour;
    
    point_t pt1,pt2,pt3,pt4;

	if ( is_fog_on() == False ) {
        return;
    }
    
    set_gl_options( FOG_PLANE );
    
    get_course_dimensions( &course_width, &course_length );
    course_angle = get_course_angle();
    slope = tan( ANGLES_TO_RADIANS( course_angle ) );
    
    left_edge_plane = make_plane( 1.0, 0.0, 0.0, 0.0 );
    
    right_edge_plane = make_plane( -1.0, 0.0, 0.0, course_width );
    
    far_clip_plane = get_far_clip_plane();
    left_clip_plane = get_left_clip_plane();
    right_clip_plane = get_right_clip_plane();
    bottom_clip_plane = get_bottom_clip_plane();
    
    bottom_plane.nml = make_vector( 0.0, 1, -slope );
    height = get_terrain_base_height( 0 );
    
    bottom_plane.d = -height * bottom_plane.nml.y;
    
    top_plane.nml = bottom_plane.nml;
    height = get_terrain_max_height( 0 );
    top_plane.d = -height * top_plane.nml.y;

    
    if(get_player_data(local_player())->view.mode == TUXEYE)
    {
        bottom_clip_plane.nml = make_vector( 0.0, 1, -slope );
        height = get_terrain_base_height( 0 ) - 40;
        bottom_clip_plane.d = -height * bottom_clip_plane.nml.y;
    }

    if ( !intersect_planes( bottom_plane, far_clip_plane, left_clip_plane, &left_pt ) )
        return;
    
    if ( !intersect_planes( bottom_plane, far_clip_plane, right_clip_plane, &right_pt ) )
        return;
    
    if ( !intersect_planes( top_plane, far_clip_plane, left_clip_plane, &top_left_pt ) )
        return;
    
    if ( !intersect_planes( top_plane, far_clip_plane, right_clip_plane, &top_right_pt ) )
        return;

    if ( !intersect_planes( bottom_clip_plane, far_clip_plane, left_clip_plane, &bottom_left_pt ) )
        return;
    
    if ( !intersect_planes( bottom_clip_plane, far_clip_plane, right_clip_plane, &bottom_right_pt ) )
        return;

    left_vec = subtract_points( top_left_pt, left_pt );
    right_vec = subtract_points( top_right_pt, right_pt );
    
    set_gl_options( FOG_PLANE );
    
    fog_colour = get_fog_colour();
    
    glColor4f( fog_colour[0], fog_colour[1], fog_colour[2], fog_colour[3] );
        
    pt1 = move_point( top_left_pt, left_vec );
    pt2 = move_point( top_right_pt, right_vec );
    pt3 = move_point( top_left_pt, scale_vector( 3.0, left_vec ) );
    pt4 = move_point( top_right_pt, scale_vector( 3.0, right_vec ) );
    
	{
    const GLfloat verticesFog []=
    {
        bottom_left_pt.x, bottom_left_pt.y, bottom_left_pt.z,
        bottom_right_pt.x, bottom_right_pt.y, bottom_right_pt.z,
        left_pt.x, left_pt.y, left_pt.z,
        right_pt.x, right_pt.y, right_pt.z,
        
        top_left_pt.x, top_left_pt.y, top_left_pt.z,
        top_right_pt.x, top_right_pt.y, top_right_pt.z,
        
        pt1.x, pt1.y, pt1.z,
        pt2.x, pt2.y, pt2.z,
        pt3.x, pt3.y, pt3.z,
        pt4.x, pt4.y, pt4.z	
    };
    
    const GLfloat colorsFog []=
    {
        fog_colour[0], fog_colour[1], fog_colour[2], fog_colour[3],
        fog_colour[0], fog_colour[1], fog_colour[2], fog_colour[3],
        fog_colour[0], fog_colour[1], fog_colour[2], fog_colour[3],
        fog_colour[0], fog_colour[1], fog_colour[2], fog_colour[3],
        fog_colour[0], fog_colour[1], fog_colour[2], 0.9 ,
        fog_colour[0], fog_colour[1], fog_colour[2], 0.9 ,
        fog_colour[0], fog_colour[1], fog_colour[2], 0.3 ,
        fog_colour[0], fog_colour[1], fog_colour[2], 0.3 ,
        fog_colour[0], fog_colour[1], fog_colour[2], 0.0 ,
        fog_colour[0], fog_colour[1], fog_colour[2], 0.0 ,	
    };
    
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_COLOR_ARRAY);
    //  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer (3, GL_FLOAT , 0, verticesFog);	
    glColorPointer(4, GL_FLOAT, 0, colorsFog);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState (GL_VERTEX_ARRAY);
	}
     */
}

void init_trees_vbo()
{
    tree_t *treeLocs;
    int numTrees;
    scalar_t treeRadius;
    scalar_t treeHeight;
    vector_t  normal;
    
    int i, vertex;
    GLfloat* treeBuffer;
    
    static const GLfloat verticesTemplateTree []=
    {
        -1, 0, 0,
        -1, 1, 0,
        1, 1, 0,
        1, 1, 0,
        1, 0, 0,
        -1, 0, 0,
        
        0, 0, -1,
        0, 1, -1,
        0, 1, 1,
        0, 1, 1,
        0, 0, 1,
        0, 0, -1,
        
    };
    static const GLfloat texCoordsTree []=
    {
        0, 0,
		0, 1,
		1, 1,
		1, 1,
		1, 0,
		0, 0,
        
        0, 0,
		0, 1,
		1, 1,
		1, 1,
		1, 0,
		0, 0,
        
    };
    
    if (trees_vbo)
    {
        glDeleteBuffers(1, &trees_vbo);
        trees_vbo=0;
    }
    
    treeLocs = treeLocsOrderedByZ;
    numTrees = get_num_trees();
    
    glGenBuffers(1, &trees_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, trees_vbo);
    
    //12 vertices per tree * 8 values per vertex
    //96 total floats per tree
    treeBuffer=(GLfloat*)malloc(numTrees*12*8*sizeof(GLfloat));
    
    for (i = 0; i < numTrees; i++)
    {
        float xOffset=treeLocs[i].ray.pt.x;
        float yOffset=treeLocs[i].ray.pt.y;
        float zOffset=treeLocs[i].ray.pt.z;
        
        treeRadius = treeLocs[i].diam/2.;
        treeHeight = treeLocs[i].height;
        
        normal = subtract_points( eye_pt, treeLocs[i].ray.pt );
        normalize_vector( &normal );
        
        for (vertex=0; vertex<12; vertex++)
        {
#define COMPONENT(value) (treeBuffer[(value)+i*96+vertex*8])
#define POSITION(value) COMPONENT(value)
#define NORMAL(value) COMPONENT((value)+3)
#define TEXCOORD(value) COMPONENT((value)+6)
            
            POSITION(0)=xOffset+verticesTemplateTree[vertex*3]*treeRadius;
            POSITION(1)=yOffset+verticesTemplateTree[1+vertex*3]*treeHeight;
            POSITION(2)=zOffset+verticesTemplateTree[2+vertex*3]*treeRadius;
            
            NORMAL(0)=normal.x;
            NORMAL(1)=normal.y;
            NORMAL(2)=normal.z;
            
            TEXCOORD(0)=((float)get_tree_index(treeLocs[i].tree_type))/3.0f+texCoordsTree[vertex*2]/3.0f;
            TEXCOORD(1)=texCoordsTree[1+vertex*2];
            
#undef COMPONENT
#undef POSITION
#undef NORMAL
#undef TEXCOORD
        }
    }
    
    glBufferData(GL_ARRAY_BUFFER, numTrees*12*8*sizeof(GLfloat), treeBuffer, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    free(treeBuffer);
}

void course_render_init() {
    int numTrees;
    tree_t* treeLocs;
    free(treeLocsOrderedByZ);
    
    treeLocs = get_tree_locs();
    numTrees = get_num_trees();
    treeLocsOrderedByZ = (tree_t*)malloc(sizeof(tree_t)*numTrees);
    memcpy(treeLocsOrderedByZ,treeLocs,sizeof(tree_t)*numTrees);
    order_trees_by_z(treeLocsOrderedByZ,numTrees);
    
    init_trees_vbo();
}