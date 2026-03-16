$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(screenTexture, 0);
SAMPLER2D(noiseTexture,  1);// @texture GameData/textures/noise/grainy5_256.png
SAMPLER2D(LutTexture,    2);// @texture GameData/textures/pp/main.png

uniform vec4 screenResolution;    // .xy
// textureSize() is unavailable in bgfx's cross-platform dialect;
// pass sizes from the application instead.
uniform vec4 noiseTexture_size;    // .xy = width, height
uniform vec4 LutTexture_size;      // .x  = height (N)  — width = N*N

#define FXAA_SPAN_MAX     4.0
#define FXAA_REDUCE_MUL   (1.0 / 4.0)
#ifndef FXAA_REDUCE_MIN
    #define FXAA_REDUCE_MIN   (1.0 / 128.0)
#endif

// Bayer 4x4 — static array initializers are not portable in bgfx (HLSL path
// rejects them), so use an explicit index function instead.
float bayer4x4(int index)
{
    if (index ==  0) return  0.0 / 16.0;
    if (index ==  1) return  8.0 / 16.0;
    if (index ==  2) return  2.0 / 16.0;
    if (index ==  3) return 10.0 / 16.0;
    if (index ==  4) return 12.0 / 16.0;
    if (index ==  5) return  4.0 / 16.0;
    if (index ==  6) return 14.0 / 16.0;
    if (index ==  7) return  6.0 / 16.0;
    if (index ==  8) return  3.0 / 16.0;
    if (index ==  9) return 11.0 / 16.0;
    if (index == 10) return  1.0 / 16.0;
    if (index == 11) return  9.0 / 16.0;
    if (index == 12) return 15.0 / 16.0;
    if (index == 13) return  7.0 / 16.0;
    if (index == 14) return 13.0 / 16.0;
    return 5.0 / 16.0; // index == 15
}

// ---------------------------------------------------------------------------
// FXAA
// ---------------------------------------------------------------------------
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
    vec3 rgbNW = texture2DLod(tex, v_rgbNW, 0.0).xyz;
    vec3 rgbNE = texture2DLod(tex, v_rgbNE, 0.0).xyz;
    vec3 rgbSW = texture2DLod(tex, v_rgbSW, 0.0).xyz;
    vec3 rgbSE = texture2DLod(tex, v_rgbSE, 0.0).xyz;
    vec4 texColor = texture2DLod(tex, v_rgbM, 0.0);
    vec3 rgbM  = texColor.xyz;

    vec3  luma   = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * inverseVP;

    vec3 rgbA = 0.5 * (
        texture2DLod(tex, fragCoord * inverseVP + dir * (1.0/3.0 - 0.5), 0.0).xyz +
        texture2DLod(tex, fragCoord * inverseVP + dir * (2.0/3.0 - 0.5), 0.0).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture2DLod(tex, fragCoord * inverseVP + dir * -0.5, 0.0).xyz +
        texture2DLod(tex, fragCoord * inverseVP + dir *  0.5, 0.0).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        return vec4(rgbA, texColor.a);
    else
        return vec4(rgbB, texColor.a);
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
    vec2 inverseVP = 1.0 / resolution.xy;
    v_rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
    v_rgbNE = (fragCoord + vec2( 1.0, -1.0)) * inverseVP;
    v_rgbSW = (fragCoord + vec2(-1.0,  1.0)) * inverseVP;
    v_rgbSE = (fragCoord + vec2( 1.0,  1.0)) * inverseVP;
    v_rgbM  = fragCoord * inverseVP;
}

vec4 applyFxaa(sampler2D tex, vec2 fragCoord, vec2 resolution) {
    vec2 v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM;
    texcoords(fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
    return fxaa(tex, fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
}

// ---------------------------------------------------------------------------
// Noise / posterize
// ---------------------------------------------------------------------------
float smoothNoise(vec2 uv)
{
    return texture2D(noiseTexture, uv).r;
}

float getAspectRatio()
{
    return noiseTexture_size.x / noiseTexture_size.y;
}

vec3 smoothPosterize(vec3 color, float steps, float softness, vec2 uv)
{
    float n = (smoothNoise(uv * 2.1) - 0.5) / steps * 0.7;
    color += n;
    color *= steps;
    vec3 floored = floor(color);
    vec3 fr = smoothstep(0.5 - softness, 0.5 + softness, fract(color));
    return (floored + fr) / steps;
}

// ---------------------------------------------------------------------------
// LUT
// ---------------------------------------------------------------------------
vec3 GetFromLUT(vec3 color)
{
    color = clamp(color, 0.0, 1.0);
 
    float N        = LutTexture_size.y;   // height of the LUT texture = slice size N
    float maxColor = N - 1.0;
 
    float cell = floor(color.b * maxColor);
    float u    = (cell * N + color.r * maxColor + 0.5) / (N * N);
    float v    = (color.g * maxColor + 0.5) / N;
 
    return texture2DLod(LutTexture, vec2(u, v), 0.0).rgb;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
void main()
{

    int x = int(mod(gl_FragCoord.x, 4.0));
    int y = int(mod(gl_FragCoord.y, 4.0));
    float bayer_value = bayer4x4(y * 4 + x);

    float aspectRatio = getAspectRatio();

    vec2 res = screenResolution.xy;

    vec3 color = applyFxaa(screenTexture, gl_FragCoord.xy, res).rgb;

    color = GetFromLUT(color);

    color = smoothPosterize(color, 70.0, 0.35, v_texcoord0 * vec2(aspectRatio, 1.0));

    color += bayer_value / 256.0;

    gl_FragColor = vec4(color, 1.0);
}
