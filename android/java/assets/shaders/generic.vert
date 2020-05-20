attribute vec4 vert_pos;
attribute vec2 source_tex_coord;

uniform mat4 MVP_mat;
uniform float course_angle;

varying vec2 dest_tex_coord;
varying float course_height;

void main()
{
    dest_tex_coord=source_tex_coord;
    course_height=vert_pos.y-sin(course_angle)*vert_pos.z;
	gl_Position=MVP_mat*vert_pos;
}