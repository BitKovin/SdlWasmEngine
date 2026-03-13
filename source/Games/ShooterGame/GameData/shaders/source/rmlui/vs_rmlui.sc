$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_position

/*
 * vs_rmlui.sc — Main vertex shader for RmlUI geometry
 *
 * Transforms vertex positions by the combined model-view-projection matrix.
 * Passes through vertex colour (premultiplied alpha) and texture coordinates.
 */

#include <bgfx_shader.sh>

uniform mat4 u_transform;

void main()
{
    vec4 pos = mul(u_transform, vec4(a_position, 0.0, 1.0));
    gl_Position = pos;

    v_color0    = a_color0;
    v_texcoord0 = a_texcoord0;
    v_position  = a_position;
}
