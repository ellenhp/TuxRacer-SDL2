precision mediump float;

uniform sampler2D texture;
uniform vec4 uniform_color;
uniform vec4 fog_color;

varying vec2 dest_tex_coord;
varying float course_height;

void main()
{
    float fog_factor=clamp(-(course_height-5.0)/10.0, 0.0, 1.0);
    gl_FragColor=mix(uniform_color*texture2D(texture, dest_tex_coord), fog_color, fog_factor);
}