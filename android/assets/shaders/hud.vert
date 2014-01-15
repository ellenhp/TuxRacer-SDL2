attribute vec4 vert_pos;
attribute vec2 source_tex_coord;

varying vec2 dest_tex_coord;

void main()
{
    dest_tex_coord=source_tex_coord;
	gl_Position=vert_pos;
}