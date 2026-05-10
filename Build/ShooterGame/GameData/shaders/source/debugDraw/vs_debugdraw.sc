$input  a_position, a_color0
$output v_color0

#include <bgfx_shader.sh>

void main()
{
    // u_modelViewProj is a bgfx built-in set via bgfx::setViewTransform /
    // bgfx::setTransform.  We rely on the identity model matrix (no
    // bgfx::setTransform call) so this is effectively viewProj * worldPos.
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_color0    = a_color0;
}
