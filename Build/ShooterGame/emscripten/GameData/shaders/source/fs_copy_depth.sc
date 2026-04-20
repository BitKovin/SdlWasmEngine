$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(depthTexture, 0);

void main()
{
    vec2 uv = v_texcoord0;
    #if !BGFX_SHADER_LANGUAGE_GLSL
    uv.y = 1.0 - uv.y;
    #endif
    gl_FragDepth = texture2D(depthTexture, uv).r;
}