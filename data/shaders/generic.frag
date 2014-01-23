precision mediump float;

uniform sampler2D texture;
uniform vec4 uniform_color;

varying vec2 dest_tex_coord;

void main()
{
    gl_FragColor=uniform_color*texture2D(texture, dest_tex_coord);
}