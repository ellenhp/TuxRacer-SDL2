attribute vec4 vert_pos;
attribute vec3 source_normal;
attribute vec2 source_tex_coord;
attribute vec3 source_terrain;

uniform mat4 MVP_mat;
uniform vec3 eye_pos;

varying vec2 dest_tex_coord;
varying vec3 dest_normal;
varying vec3 transparencies;

void main()
{
    dest_tex_coord=source_tex_coord;
    transparencies=source_terrain;
    dest_normal=source_normal;
	gl_Position=MVP_mat*vert_pos;
}