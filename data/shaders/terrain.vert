attribute vec4 vert_pos;
attribute vec3 source_normal;
attribute vec2 source_tex_coord;
attribute vec3 source_terrain;

uniform mat4 MVP_mat;
uniform vec3 eye_pos;
uniform vec3 tux_pos;
uniform float forward_clip;

varying vec2 dest_env_coord;
varying vec2 dest_tex_coord;
varying vec3 dest_normal;
varying vec3 transparencies;
varying float fog_factor;

void main()
{
    vec3 toMap=normalize(reflect(vert_pos.xyz-eye_pos, source_normal));
    dest_env_coord=vec2(atan(toMap.z,toMap.x)/3.14, toMap.y/2.0+0.5);
    
    dest_tex_coord=source_tex_coord;
    transparencies=source_terrain;
    dest_normal=source_normal;
    fog_factor=clamp((tux_pos.z-vert_pos.z)/forward_clip, 0.0, 1.0);
    //fog_factor=fog_factor*fog_factor;
    gl_Position=MVP_mat*vert_pos;
}