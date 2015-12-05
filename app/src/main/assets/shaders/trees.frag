precision mediump float;

uniform sampler2D texture;
uniform vec4 uniform_color;

uniform vec4 fog_color;

varying vec2 dest_tex_coord;
varying float fog_factor;

void main()
{
    if (texture2D(texture, dest_tex_coord).a<0.01)
    {
        discard;
    }
    gl_FragColor=mix(uniform_color*texture2D(texture, dest_tex_coord), fog_color, fog_factor);
}