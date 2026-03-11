$input a_position, a_texcoord0
$output v_texcoord0

#include <bgfx_shader.sh>

uniform mat4 u_Model;
uniform mat4 u_Projection;

void main()
{
    v_texcoord0 = a_texcoord0;
    gl_Position = mul(u_Projection, mul(u_Model, vec4(a_position.xy, 0.0, 1.0)));
}