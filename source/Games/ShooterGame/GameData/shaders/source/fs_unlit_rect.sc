$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1

#include <bgfx_shader.sh>

SAMPLER2D(u_texture, 0);

uniform vec4 modelColor;
uniform vec4 u_atlasRect;   // minU, minV, maxU, maxV  (normalized [0..1])

void main()
{
    // remap the full [0,1] texcoord of the plane into the atlas sub-rectangle
    vec2 uv = mix(u_atlasRect.xy, u_atlasRect.zw, v_texcoord0);

    vec4 texColor = texture2D(u_texture, uv) * v_color0 * modelColor;

    float alpha = texColor.a;

    if (alpha < 0.01)
        discard;

    gl_FragColor = vec4(texColor.rgb, alpha);
}