attribute vec4 vert_pos;
attribute vec3 source_normal;

uniform mat4 MVP_mat;

varying vec3 dest_normal;

void main()
{
	gl_Position=MVP_mat*vert_pos;
}