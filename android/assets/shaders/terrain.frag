precision mediump float;

uniform sampler2D terrain0;
uniform sampler2D terrain1;
uniform sampler2D terrain2;
uniform sampler2D texture;
uniform vec4 uniform_color;

varying vec2 dest_tex_coord;
varying vec3 transparencies;

void main()
{
    vec4 color0=texture2D(terrain0, dest_tex_coord);
    vec4 color1=texture2D(terrain1, dest_tex_coord);
    vec4 color2=texture2D(terrain2, dest_tex_coord);
    vec4 terrain_color=color0*transparencies[0]+color1*transparencies[1]+color2*transparencies[2];
    terrain_color.a=1.0;
    gl_FragColor=terrain_color;
}