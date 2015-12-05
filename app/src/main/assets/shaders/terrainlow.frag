precision mediump float;

uniform sampler2D terrain0;
uniform sampler2D terrain1;
uniform sampler2D terrain2;
uniform vec4 uniform_color;

uniform vec3 lightpos;
uniform vec4 lightspecular;
uniform vec4 lightdiffuse;
uniform vec4 lightambient;

varying vec2 dest_tex_coord;
varying vec3 dest_normal;
varying vec3 transparencies;

void main()
{
    vec4 color0=texture2D(terrain0, dest_tex_coord);
    vec4 color1=texture2D(terrain1, dest_tex_coord);
    vec4 color2=texture2D(terrain2, dest_tex_coord);
    vec4 terrain_color=color0*transparencies[0]+color1*transparencies[1]+color2*transparencies[2];
    
    float diffuse_intensity=dot(dest_normal, lightpos);
    
    vec4 color=(diffuse_intensity * lightdiffuse + lightambient) * terrain_color;
    
    color.a=1.0;
    
    gl_FragColor=color;
}