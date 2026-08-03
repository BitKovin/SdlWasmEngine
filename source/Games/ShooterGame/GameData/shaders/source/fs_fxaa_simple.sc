$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(screenTexture, 0);

uniform vec4 screenSize;

#define FXAA_SPAN_MAX       2.0
#define FXAA_REDUCE_MUL     (1.0 / 4.0)
#define FXAA_REDUCE_MIN     (1.0 / 32.0)

// Number of taps taken along the blur direction on each side of center.
// Total samples = FXAA_SAMPLE_HALF * 2 + 1  (the +1 is the center tap).
//   8  → 17 taps  — default, smooth edges with no dithering
//  16  → 33 taps  — very smooth, higher GPU cost
//   4  → 9 taps   — cheaper, still better than original
#define FXAA_SAMPLE_HALF    4

vec4 fxaa(
    sampler2D tex,
    vec2 fragCoord,
    vec2 resolution,
    vec2 v_rgbNW,
    vec2 v_rgbNE,
    vec2 v_rgbSW,
    vec2 v_rgbSE,
    vec2 v_rgbM
) {
    vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);

    vec3 rgbNW = texture2D(tex, v_rgbNW).xyz;
    vec3 rgbNE = texture2D(tex, v_rgbNE).xyz;
    vec3 rgbSW = texture2D(tex, v_rgbSW).xyz;
    vec3 rgbSE = texture2D(tex, v_rgbSE).xyz;
    vec4 texColor = texture2D(tex, v_rgbM);
    vec3 rgbM  = texColor.xyz;

    vec3  luma    = vec3(0.299, 0.587, 0.114);
    float lumaNW  = dot(rgbNW, luma);
    float lumaNE  = dot(rgbNE, luma);
    float lumaSW  = dot(rgbSW, luma);
    float lumaSE  = dot(rgbSE, luma);
    float lumaM   = dot(rgbM,  luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir = vec2(0.0, 0.0);
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * inverseVP;

    // Accumulate FXAA_SAMPLE_HALF taps on each side plus the center.
    // Offsets are evenly distributed in [-0.5, +0.5] so coverage always
    // equals SPAN_MAX pixels regardless of FXAA_SAMPLE_HALF.
    // step = 1.0 / FXAA_SAMPLE_HALF  (half-open on each end)
    vec4 acc  = vec4(0.0, 0.0, 0.0, 0.0);
    float n   = float(FXAA_SAMPLE_HALF);
    float invN = 1.0 / n;

    for (int i = 0; i < FXAA_SAMPLE_HALF; ++i)
    {
        float t = (float(i) + 0.5) * invN * 0.5;   // 0.5/n … 0.5-0.5/n
        acc += texture2D(tex, fragCoord * inverseVP - dir * t);
        acc += texture2D(tex, fragCoord * inverseVP + dir * t);
    }
    // Center tap
    acc += texture2D(tex, fragCoord * inverseVP);

    float totalSamples = float(FXAA_SAMPLE_HALF) * 2.0 + 1.0;
    vec4 rgbFull = acc / totalSamples;

    // Luma guard: if the wide average overshoots the local range it has
    // crossed a different edge — fall back to just the center tap.
    float lumaFull = dot(rgbFull.xyz, luma);
    if ((lumaFull < lumaMin) || (lumaFull > lumaMax))
        return texColor;
    else
        return rgbFull;
}

void texcoords(
    vec2 fragCoord,
    vec2 resolution,
    out vec2 v_rgbNW,
    out vec2 v_rgbNE,
    out vec2 v_rgbSW,
    out vec2 v_rgbSE,
    out vec2 v_rgbM
) {
    vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);
    v_rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
    v_rgbNE = (fragCoord + vec2( 1.0, -1.0)) * inverseVP;
    v_rgbSW = (fragCoord + vec2(-1.0,  1.0)) * inverseVP;
    v_rgbSE = (fragCoord + vec2( 1.0,  1.0)) * inverseVP;
    v_rgbM  = fragCoord * inverseVP;
}

vec4 applyFxaa(sampler2D tex, vec2 fragCoord, vec2 resolution)
{
    vec2 v_rgbNW = vec2(0.0, 0.0);
    vec2 v_rgbNE = vec2(0.0, 0.0);
    vec2 v_rgbSW = vec2(0.0, 0.0);
    vec2 v_rgbSE = vec2(0.0, 0.0);
    vec2 v_rgbM  = vec2(0.0, 0.0);
    texcoords(fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
    return fxaa(tex, fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
}

void main()
{
    gl_FragColor = applyFxaa(screenTexture, gl_FragCoord.xy, screenSize.xy);
}