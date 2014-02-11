attribute vec4 vert_pos;
attribute vec2 source_tex_coord;

uniform mat4 MVP_mat;
uniform vec3 tux_pos;
uniform float forward_clip;

varying vec2 dest_tex_coord;
varying float fog_factor;

void main()
{
    dest_tex_coord=source_tex_coord;
    fog_factor=clamp((tux_pos.z-vert_pos.z)/forward_clip, 0.0, 1.0);
	gl_Position=MVP_mat*vert_pos;
}