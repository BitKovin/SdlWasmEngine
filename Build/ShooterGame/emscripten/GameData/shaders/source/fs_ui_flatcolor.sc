$input v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 u_Color;

void main()
{
    gl_FragColor = u_Color;
}
