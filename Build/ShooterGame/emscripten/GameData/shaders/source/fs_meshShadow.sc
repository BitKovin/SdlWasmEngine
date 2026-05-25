$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

#include <bgfx_shader.sh>

uniform vec4 u_color;

void main() 
{

    gl_FragColor = u_color;
}