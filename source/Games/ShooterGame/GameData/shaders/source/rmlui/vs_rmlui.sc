$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <../bgfx_shader.sh>

uniform mat4 _transform;

void main()
{
    v_texcoord0 = a_texcoord0;
    v_color0    = a_color0;

    // FIX: _translate has been removed. Translation is now baked into
    // _transform on the CPU side as (projection * css_transform * translate(offset)).
    // The separate _translate vec4 uniform was silently ignored at runtime because
    // Shader::SetUniform has no vec4 overload, leaving _translate = (0,0,0,0)
    // and causing all geometry to render at document position (0,0) = top-left.
    gl_Position = mul(_transform, vec4(a_position.xy, 0.0, 1.0));
}