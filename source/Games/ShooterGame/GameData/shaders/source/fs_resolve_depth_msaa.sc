$input v_texcoord0

#include <bgfx_shader.sh>

#if BGFX_SHADER_LANGUAGE_GLSL && (BGFX_SHADER_LANGUAGE_GLSL == 300)
// ❌ GLES 2.0 / WebGL 1 — no MSAA support at all

void main()
{
    // Output dummy depth (or just do nothing meaningful)
    gl_FragDepth = 1.0;
}

#else

// MSAA depth texture (only valid on supported backends)
SAMPLER2DMS(depthMsaa, 0);

uniform vec4 uResolution;
uniform vec4 uSampleCount;

void main()
{
    vec2 uv = v_texcoord0;

    #if !BGFX_SHADER_LANGUAGE_GLSL
    uv.y = 1.0 - uv.y;
    #endif

    ivec2 texel = ivec2(uv * uResolution.xy);

    float depth = texelFetch(depthMsaa, texel, 0).r;

    int count = int(uSampleCount.x);

    for (int i = 1; i < count; ++i)
    {
        float sampleDepth = texelFetch(depthMsaa, texel, i).r;
        depth = min(depth, sampleDepth);
    }

    gl_FragDepth = depth;
}

#endif