#include "tuxracer.h"
#include "shaders.h"
#include "winsys.h"
#include "SDL.h"

#define MAX_SHADER_SIZE 10000

static GLuint generic_program;
static GLuint terrain_program;

static GLuint active_program;

static bool_t programs_initialized=False;

int load_shader(GLenum type, char* filename)
{
    GLuint shader;
    size_t chars_read;
    SDL_RWops* file;
    GLint compiled;
    char* shader_source=malloc(MAX_SHADER_SIZE);
   
    shader=glCreateShader(type);
    
    file=SDL_RWFromFile(filename, "r");

    if (file==NULL)
    {
        print_debug(DEBUG_OTHER, "shader %s wasn't loaded", filename);
        winsys_exit(1);
    }
    
    chars_read=SDL_RWread(file, shader_source, 1, MAX_SHADER_SIZE-1);
    
    glShaderSource(shader, 1, (const char**)(&shader_source), &chars_read);
    
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled==GL_FALSE)
    {
        print_debug(DEBUG_OTHER, "shader %s failed to compile", filename);
        winsys_exit(1);
    }
    
    free(shader_source);
    
    return shader;
}

void init_programs()
{
    GLint linked;

    if (programs_initialized)
    {
        return;
    }
    
    generic_program=glCreateProgram();

    glAttachShader(generic_program, load_shader(GL_VERTEX_SHADER, "shaders/generic.vert"));
    glAttachShader(generic_program, load_shader(GL_FRAGMENT_SHADER, "shaders/generic.frag"));
    
    glLinkProgram(generic_program);
    
    glGetProgramiv(generic_program, GL_LINK_STATUS, &linked);
    if (linked==GL_FALSE)
    {
        print_debug(DEBUG_OTHER, "shader failed to link");
        winsys_exit(1);
    }
    
    programs_initialized=True;
}

void use_terrain_program()
{
}

void use_generic_program()
{
    if (programs_initialized)
    {
        glUseProgram(generic_program);
        active_program=generic_program;
    }
}

GLuint shader_get_attrib_location(char* name)
{
    return glGetAttribLocation(active_program, name);
}

void shader_set_texture(GLuint texture)
{
    glUniform1i(glGetUniformLocation(active_program, "texture"), texture);
}

void shader_set_color(GLfloat* argb)
{
    glUniform4fv(glGetUniformLocation(active_program, "uniform_color"), 1, argb);
}

