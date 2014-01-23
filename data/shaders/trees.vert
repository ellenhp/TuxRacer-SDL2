attribute vec4 vert_pos;
attribute vec2 source_tex_coord;

uniform mat4 MVP_mat;

varying vec2 dest_tex_coord;

void main()
{
    dest_tex_coord=source_tex_coord;
	gl_Position=MVP_mat*vert_pos;
}