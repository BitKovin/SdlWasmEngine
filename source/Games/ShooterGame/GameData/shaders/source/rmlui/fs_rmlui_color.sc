$input v_color0, v_texcoord0
$output gl_FragColor

#include <../bgfx_shader.sh>

void main()
{
    gl_FragColor = v_color0;
}