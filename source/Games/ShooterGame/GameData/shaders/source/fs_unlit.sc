$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1

#include <bgfx_shader.sh>

SAMPLER2D(u_texture, 0);

void main()
{
    vec4 texColor = texture2D(u_texture, v_texcoord0) * v_color0;

    float alpha = texColor.a;

    if (alpha < 0.01)
        discard;

    gl_FragColor = vec4(texColor.rgb, alpha);
}
