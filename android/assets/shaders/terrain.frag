precision mediump float;

uniform sampler2D terrains[3];
uniform sampler2D texture;
uniform vec4 uniform_color;

varying vec2 dest_tex_coord;
varying vec3 transparencies;

void main()
{
    bool is_terrain=true;
    float cutoff=0.1;
    if (transparencies[0]<cutoff && transparencies[1]<cutoff && transparencies[2]<cutoff)
    {
        is_terrain=false;
    }
    if (is_terrain)
    {
        /*vec4 color0=texture2D(terrains[0], dest_tex_coord);
        vec4 color1=texture2D(terrains[1], dest_tex_coord);
        vec4 color2=texture2D(terrains[2], dest_tex_coord);
        vec4 terrain_color=color0*transparencies[0]+color1*transparencies[1]+color2*transparencies[2];*/
        gl_FragColor=vec4(1.0, 0.0, 0.0, 1.0);
    }
    else
    {
        if (texture2D(texture, dest_tex_coord).a<0.01)
        {
            discard;
        }
        gl_FragColor=texture2D(texture, dest_tex_coord);
    }
}