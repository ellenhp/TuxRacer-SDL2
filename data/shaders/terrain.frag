precision mediump float;

uniform sampler2D terrain0;
uniform sampler2D terrain1;
uniform sampler2D terrain2;
uniform sampler2D envmap;
uniform vec4 uniform_color;

uniform vec3 lightpos;
uniform vec4 lightspecular;
uniform vec4 lightdiffuse;
uniform vec4 lightambient;

uniform vec4 fog_color;

varying vec2 dest_env_coord;
varying vec2 dest_tex_coord;
varying vec3 dest_normal;
varying vec3 transparencies;
varying float fog_factor;

void main()
{
    vec4 color_envmap=texture2D(envmap, dest_env_coord);
    
    vec4 color0=(texture2D(terrain0, dest_tex_coord)+color_envmap);
    vec4 color1=texture2D(terrain1, dest_tex_coord);
    vec4 color2=texture2D(terrain2, dest_tex_coord);
    vec4 terrain_color=color0*transparencies[0]+color1*transparencies[1]+color2*transparencies[2];
    
    float diffuse_intensity=dot(dest_normal, lightpos);
    
    vec4 light=diffuse_intensity * lightdiffuse + lightambient;
    vec4 color=mix(light*terrain_color, fog_color, fog_factor);
    
    color.a=1.0;
    
    gl_FragColor=color;
}