#include "tuxracer.h"
#include "shaders.h"
#include "winsys.h"
#include "gl_util.h"
#include "course_load.h"
#include "SDL.h"

#define MAX_SHADER_SIZE 10000

static GLuint generic_program;
static GLuint hud_program;
static GLuint terrain_program;
static GLuint tux_program;

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

void init_shader_program(GLuint* program, char* vertfile, char* fragfile)
{
    GLint linked;

    *program=glCreateProgram();
    
    glAttachShader(*program, load_shader(GL_VERTEX_SHADER, vertfile));
    glAttachShader(*program, load_shader(GL_FRAGMENT_SHADER, fragfile));
    
    glLinkProgram(*program);
    
    glGetProgramiv(*program, GL_LINK_STATUS, &linked);
    if (linked==GL_FALSE)
    {
        print_debug(DEBUG_OTHER, "shader failed to link");
        winsys_exit(1);
    }
}

void init_programs()
{
    if (programs_initialized)
    {
        return;
    }
    
    init_shader_program(&generic_program, "shaders/generic.vert", "shaders/generic.frag");
    init_shader_program(&hud_program, "shaders/hud.vert", "shaders/hud.frag");
    init_shader_program(&terrain_program, "shaders/terrain.vert", "shaders/terrain.frag");
    init_shader_program(&tux_program, "shaders/tux.vert", "shaders/tux.frag");
    
    programs_initialized=True;
}

void use_terrain_program()
{
    if (programs_initialized && active_program!=terrain_program)
    {
        glUseProgram(terrain_program);
        active_program=terrain_program;
        set_MVP();
        set_light_uniforms();
    }
}

void use_generic_program()
{
    if (programs_initialized && active_program!=generic_program)
    {
        glUseProgram(generic_program);
        active_program=generic_program;
        set_MVP();
    }
}

void use_hud_program()
{
    if (programs_initialized && active_program!=hud_program)
    {
        glUseProgram(hud_program);
        active_program=hud_program;
        set_MVP();
    }
}

void use_tux_program()
{
    if (programs_initialized && active_program!=tux_program)
    {
        glUseProgram(tux_program);
        active_program=tux_program;
        set_MVP();
    }
}

GLuint shader_get_attrib_location(char* name)
{
    return glGetAttribLocation(active_program, name);
}

GLuint shader_get_uniform_location(char* name)
{
    return glGetUniformLocation(active_program, name);
}

void shader_set_texture(GLuint texture)
{
    glUniform1i(glGetUniformLocation(active_program, "texture"), texture);
}

void shader_set_color(GLfloat* argb)
{
    glUniform4fv(glGetUniformLocation(active_program, "uniform_color"), 1, argb);
}

