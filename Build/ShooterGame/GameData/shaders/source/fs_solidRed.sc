$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

#include <bgfx_shader.sh>

void main()
{
    vec4 texColor = vec4(1.0,0.0,0.0,1.0);


    gl_FragColor = vec4(texColor);
}
