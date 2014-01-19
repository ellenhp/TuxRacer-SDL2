precision mediump float;

uniform vec3 lightpos;
uniform vec4 lightspecular;
uniform vec4 lightdiffuse;
uniform vec4 lightambient;

varying vec3 dest_normal;
varying vec3 dest_color;

void main()
{
    float diffuse_intensity=dot(dest_normal, lightpos);
    gl_FragColor= vec4((diffuse_intensity * lightdiffuse + lightambient).rgb * dest_color, 1.0)*1.5;
}