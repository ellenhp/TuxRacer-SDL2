attribute vec4 vert_pos;
attribute vec3 source_normal;
attribute vec3 source_color;

uniform mat4 MVP_mat;

varying vec3 dest_normal;
varying vec3 dest_color;

void main()
{
    dest_normal=source_normal;
    dest_color=source_color;
	gl_Position=MVP_mat*vert_pos;
}