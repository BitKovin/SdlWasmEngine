$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <../bgfx_shader.sh>

// Projection * CSS transform matrix. Updated once per SetTransform() call.
uniform mat4 _transform;

// Per-draw pixel translation from RenderGeometry()'s `translation` parameter.
// Declared as vec4 because BGFX does not support vec2/vec3 uniforms.
// Only .xy is used; .zw are padding.
uniform vec4 _translate;

void main()
{
    v_texcoord0 = a_texcoord0;
    v_color0    = a_color0;

    // Mirror the GL3 vertex shader exactly:
    //   vec2 translatedPos = inPosition + _translate;
    //   gl_Position = _transform * vec4(translatedPos, 0.0, 1.0);
    gl_Position = mul(_transform, vec4(a_position.xy + _translate.xy, 0.0, 1.0));
}