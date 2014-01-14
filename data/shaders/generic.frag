precision mediump float;

uniform sampler2D texture;

varying vec2 dest_tex_coord;

void main()
{
    if (texture2D(texture, dest_tex_coord).a<0.01)
    {
        discard;
    }
	gl_FragColor=texture2D(texture, dest_tex_coord);
	//gl_FragColor=vec4(1.0, 1.0, 1.0, 1.0);
}