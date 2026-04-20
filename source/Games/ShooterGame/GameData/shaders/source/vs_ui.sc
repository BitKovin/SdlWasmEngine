$input a_position, a_texcoord0
$output v_texcoord0

#include <bgfx_shader.sh>

uniform mat4 u_Model;
uniform mat4 u_Projection;

uniform vec4 bilboard;

void main()
{
    v_texcoord0 = a_texcoord0;

    #if BGFX_SHADER_LANGUAGE_GLSL

    #else
    if(bilboard.x > 0.5)
    {
        //v_texcoord0.y = 1.0 - v_texcoord0.y;
    }

    #endif

    gl_Position = mul(u_Projection, mul(u_Model, vec4(a_position.xy, 0.0, 1.0)));
}