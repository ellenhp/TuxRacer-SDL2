attribute vec4 vert_pos;
attribute vec2 source_tex_coord;
attribute vec3 source_terrain;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

varying vec2 dest_tex_coord;
varying vec3 transparencies;

void main()
{
    mat4 MVP_mat=projection * view * model;
    dest_tex_coord=source_tex_coord;
    transparencies=source_terrain;
	gl_Position=MVP_mat*vert_pos;
}