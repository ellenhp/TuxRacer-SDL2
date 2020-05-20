#include "course_vbo.h"
#include "multiplayer.h"
#include "course_mgr.h"
#include "course_render.h"
#include "course_load.h"
#include "shaders.h"
#include "textures.h"

//TODO is this poorly aligned?
//data array. V=position, N=normal, C=texture coord, T=terrain transperency
//VVV NNN CC TTT
#define STRIDE 9
#define STRIDE_BYTES (STRIDE * sizeof(GLfloat))

static GLuint data_vbo = 0;
static GLuint index_vbo = 0;
static GLuint course_x_size, course_z_size;

static GLushort indices[USHRT_MAX];
static GLushort num_indices;

void reset_course_vbo()
{
    if (data_vbo)
    {
        glDeleteBuffers(1, &data_vbo);
        data_vbo = 0;
    }
    if (index_vbo)
    {
        glDeleteBuffers(1, &index_vbo);
        index_vbo = 0;
    }
}

void generate_indices(int startZ)
{
#define VERTEX_NUM(x_val, z_val) ((z_val)*course_x_size + x_val - offset)
    int z, x;
    int offset = startZ * course_x_size;
    num_indices = 0;
    for (z = startZ; z < course_z_size - 1; z++)
    {
        if (num_indices + 6 * course_x_size >= USHRT_MAX)
        {
            break;
        }
        for (x = 0; x < course_x_size - 1; x++)
        {
            indices[num_indices++] = VERTEX_NUM(x, z);
            indices[num_indices++] = VERTEX_NUM(x + 1, z);
            indices[num_indices++] = VERTEX_NUM(x, z + 1);
            indices[num_indices++] = VERTEX_NUM(x + 1, z);
            indices[num_indices++] = VERTEX_NUM(x + 1, z + 1);
            indices[num_indices++] = VERTEX_NUM(x, z + 1);
        }
    }
#undef VERTEX_NUM
}

void init_course_vbo(scalar_t *elevation, terrain_t *terrain, int nx, int nz, scalar_t scalex, scalar_t scalez)
{
    vector_t *normals = get_course_normals();

    scalar_t course_width, course_length;
    get_course_dimensions(&course_width, &course_length);

    int size = nx * nz * STRIDE_BYTES;
    int z, x;

    course_x_size = nx;
    course_z_size = nz;

    GLfloat *VNT_array = (GLfloat *)malloc(size);

    int absolute_index = 0;

    reset_course_vbo();

    for (z = 0; z < nz; z++)
    {
        for (x = 0; x < nx; x++)
        {
            absolute_index = z * nx + x;

#define DATA_AT_COORD (VNT_array + absolute_index * STRIDE)
#define VERTEX_AT_COORD DATA_AT_COORD
#define NORMAL_AT_COORD (DATA_AT_COORD + 3)
#define TEXCOORD_AT_COORD (DATA_AT_COORD + 6)
#define TERRAIN_AT_COORD ((GLubyte *)(DATA_AT_COORD + 8))

            VERTEX_AT_COORD[0] = (GLfloat)x / (nx - 1.) * course_width;
            VERTEX_AT_COORD[1] = elevation[absolute_index];
            VERTEX_AT_COORD[2] = -(GLfloat)z / (nz - 1.) * course_length;

            vector_t normal = normals[absolute_index];
            NORMAL_AT_COORD[0] = normal.x;
            NORMAL_AT_COORD[1] = normal.y;
            NORMAL_AT_COORD[2] = normal.z;

            TEXCOORD_AT_COORD[0] = (GLfloat)x / (nx - 1.) * course_width / 20;
            TEXCOORD_AT_COORD[1] = (GLfloat)z / (nz - 1.) * course_length / 20;

            TERRAIN_AT_COORD[0] = 0;
            TERRAIN_AT_COORD[1] = 0;
            TERRAIN_AT_COORD[2] = 0;
            TERRAIN_AT_COORD[terrain[absolute_index]] = 1;
        }
    }

    glGenBuffers(1, &data_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, data_vbo);
    glBufferData(GL_ARRAY_BUFFER, nx * nz * STRIDE_BYTES, VNT_array, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &index_vbo);

    generate_indices(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(GLfloat), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    free(VNT_array);
}

int get_start_z()
{
    scalar_t course_width, course_length;
    get_course_dimensions(&course_width, &course_length);

    float player_z = -get_player_data(local_player())->pos.z - 30;

    int guess_z = (int)(player_z * (course_z_size - 1) / course_length);

    if (guess_z > 0)
    {
        return guess_z;
    }
    else
    {
        return 0;
    }
}

int get_num_vertices(int startZ, int *nextStartZ)
{
    int z, x;
    int indices = 0;
    scalar_t course_width, course_length;
    get_course_dimensions(&course_width, &course_length);
    for (z = startZ; z < course_z_size - 1; z++)
    {
        if (indices + 6 * course_x_size >= USHRT_MAX || ((GLfloat)(z - get_start_z()) / (course_z_size - 1.) * course_length) > getparam_forward_clip_distance() / 2)
        {
            break;
        }
        for (x = 0; x < course_x_size - 1; x++)
        {
            indices += 6;
        }
    }
    *nextStartZ = z;
    return indices;
}

void bind_textures()
{
    GLuint texobj[3];
    GLuint envmapObj;

    glActiveTexture(GL_TEXTURE0 + Ice);
    if (!get_texture_binding("ice", texobj + Ice))
    {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, texobj[Ice]);

    glActiveTexture(GL_TEXTURE0 + Rock);
    if (!get_texture_binding("rock", texobj + Rock))
    {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, texobj[Rock]);

    glActiveTexture(GL_TEXTURE0 + Snow);
    if (!get_texture_binding("snow", texobj + Snow))
    {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, texobj[Snow]);

    glActiveTexture(GL_TEXTURE0 + 3);
    if (!get_texture_binding("terrain_envmap", &envmapObj))
    {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, envmapObj);

    glUniform1i(shader_get_uniform_location(SHADER_TERRAIN_TEXTURES_NAME "0"), 0);
    glUniform1i(shader_get_uniform_location(SHADER_TERRAIN_TEXTURES_NAME "1"), 1);
    glUniform1i(shader_get_uniform_location(SHADER_TERRAIN_TEXTURES_NAME "2"), 2);

    glUniform1i(shader_get_uniform_location(SHADER_ENVMAP_NAME), 3);

    glActiveTexture(GL_TEXTURE0);
}

void unbind_textures()
{
    glActiveTexture(GL_TEXTURE0 + Ice);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0 + Rock);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0 + Snow);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
}

void draw_course_vbo()
{
    int startZ = get_start_z();
    int nextStartZ = 0;
    int numVerticies = 0;
    if (!data_vbo)
    {
        return;
    }

    bind_textures();

    glBindBuffer(GL_ARRAY_BUFFER, data_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);

    while (numVerticies = get_num_vertices(startZ, &nextStartZ))
    {
        glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 3, GL_FLOAT, GL_FALSE, STRIDE_BYTES, startZ * course_x_size * STRIDE_BYTES);
        glEnableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));

        glVertexAttribPointer(shader_get_attrib_location(SHADER_NORMAL_NAME), 3, GL_FLOAT, GL_FALSE, STRIDE_BYTES, startZ * course_x_size * STRIDE_BYTES + 3 * sizeof(GLfloat));
        glEnableVertexAttribArray(shader_get_attrib_location(SHADER_NORMAL_NAME));

        glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME), 2, GL_FLOAT, GL_FALSE, STRIDE_BYTES, startZ * course_x_size * STRIDE_BYTES + 6 * sizeof(GLfloat));
        glEnableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));

        glVertexAttribPointer(shader_get_attrib_location(SHADER_TERRAINS_NAME), 3, GL_UNSIGNED_BYTE, GL_FALSE, STRIDE_BYTES, startZ * course_x_size * STRIDE_BYTES + 8 * sizeof(GLfloat));
        glEnableVertexAttribArray(shader_get_attrib_location(SHADER_TERRAINS_NAME));

        glDrawElements(GL_TRIANGLES, numVerticies, GL_UNSIGNED_SHORT, 0);

        startZ = nextStartZ;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_NORMAL_NAME));
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));
    glDisableVertexAttribArray(shader_get_attrib_location(SHADER_TERRAINS_NAME));

    unbind_textures();
}
