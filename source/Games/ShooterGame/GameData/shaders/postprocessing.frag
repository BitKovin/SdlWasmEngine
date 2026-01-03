#version 300 es
precision highp float;

//disable_optimization

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform sampler2D noiseTexture;// @texture GameData/textures/noise/grainy5_256.png
uniform sampler2D LutTexture;// @texture GameData/textures/pp/main.png

uniform vec2 screenResolution;

/**
Basic FXAA implementation based on the code on geeks3d.com with the
modification that the texture2DLod stuff was removed since it's
unsupported by WebGL.

--

From:
https://github.com/mitsuhiko/webgl-meincraft

Copyright (c) 2011 by Armin Ronacher.

Some rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * The names of the contributors may not be used to endorse or
      promote products derived from this software without specific
      prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

    #define FXAA_SPAN_MAX     4.0

    #define FXAA_REDUCE_MUL   (1.0 / 4.0)

#ifndef FXAA_REDUCE_MIN
    #define FXAA_REDUCE_MIN   (1.0/ 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
    #define FXAA_REDUCE_MUL   (1.0 / 8.0)
#endif
#ifndef FXAA_SPAN_MAX
    #define FXAA_SPAN_MAX     8.0
#endif

//optimized version for mobile, where dependent
//texture reads can be a bottleneck
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
    vec4 color;
    mediump vec2 inverseVP = vec2(1.0f / resolution.x, 1.0f / resolution.y);
    vec3 rgbNW = textureLod(tex, v_rgbNW, 0.0).xyz;
    vec3 rgbNE = textureLod(tex, v_rgbNE, 0.0).xyz;
    vec3 rgbSW = textureLod(tex, v_rgbSW, 0.0).xyz;
    vec3 rgbSE = textureLod(tex, v_rgbSE, 0.0).xyz;
    vec4 texColor = textureLod(tex, v_rgbM, 0.0);
    vec3 rgbM = texColor.xyz;
    vec3 luma = vec3(0.299f, 0.587f, 0.114f);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    mediump vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
        (0.25f * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0f / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * inverseVP;

    vec3 rgbA = 0.5f * (textureLod(tex, fragCoord * inverseVP + dir * (1.0f / 3.0f - 0.5f), 0.0).xyz +
        textureLod(tex, fragCoord * inverseVP + dir * (2.0f / 3.0f - 0.5f), 0.0).xyz);
    vec3 rgbB = rgbA * 0.5f + 0.25f * (textureLod(tex, fragCoord * inverseVP + dir * -0.5f, 0.0).xyz +
        textureLod(tex, fragCoord * inverseVP + dir * 0.5f, 0.0).xyz);

    float lumaB = dot(rgbB, luma);
    if((lumaB < lumaMin) || (lumaB > lumaMax))
        color = vec4(rgbA, texColor.a);
    else
        color = vec4(rgbB, texColor.a);
    return color;
}

//To save 9 dependent texture reads, you can compute
//these in the vertex shader and use the optimized
//frag.glsl function in your frag shader.

//This is best suited for mobile devices, like iOS.

void texcoords(
    vec2 fragCoord,
    vec2 resolution,
    out vec2 v_rgbNW,
    out vec2 v_rgbNE,
    out vec2 v_rgbSW,
    out vec2 v_rgbSE,
    out vec2 v_rgbM
) {
    vec2 inverseVP = 1.0f / resolution.xy;
    v_rgbNW = (fragCoord + vec2(-1.0f, -1.0f)) * inverseVP;
    v_rgbNE = (fragCoord + vec2(1.0f, -1.0f)) * inverseVP;
    v_rgbSW = (fragCoord + vec2(-1.0f, 1.0f)) * inverseVP;
    v_rgbSE = (fragCoord + vec2(1.0f, 1.0f)) * inverseVP;
    v_rgbM = vec2(fragCoord * inverseVP);
}

vec4 applyFxaa(sampler2D tex, vec2 fragCoord, vec2 resolution) {
    mediump vec2 v_rgbNW;
    mediump vec2 v_rgbNE;
    mediump vec2 v_rgbSW;
    mediump vec2 v_rgbSE;
    mediump vec2 v_rgbM;

	//compute the texture coords
    texcoords(fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);

	//compute FXAA
    return fxaa(tex, fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
}

// Define a 4x4 Bayer matrix, normalized to [0, 1)
const float bayer4x4[16] = float[16](
    0.0 / 16.0,  8.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0,
   12.0 / 16.0,  4.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0,
    3.0 / 16.0, 11.0 / 16.0,  1.0 / 16.0,  9.0 / 16.0,
   15.0 / 16.0,  7.0 / 16.0, 13.0 / 16.0,  5.0 / 16.0
);

// Smoothed pseudo-random noise
float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Smooth 2D noise
float smoothNoise(vec2 uv) 
{
    return texture(noiseTexture, uv).r;
}

float getAspectRatio()
{
    ivec2 size = textureSize(noiseTexture, 0);
    return float(size.x) / float(size.y);
}

// --- Smooth posterization with large noise ---
vec3 smoothPosterize(vec3 color, float steps, float softness, vec2 uv) {
    // Add smooth large-scale noise
    float n = (smoothNoise(uv * 2.3) - 0.5) / steps * 0.6; // big noise, low frequency
    color += n;

    // Posterize with smooth transitions
    color *= steps;
    vec3 floored = floor(color);
    vec3 frac = smoothstep(0.5 - softness, 0.5 + softness, fract(color));
    return (floored + frac) / steps;
}

vec3 GetFromLUT(vec3 color)
{
    // replicate HLSL saturate
    color = clamp(color, 0.0, 1.0);

    float N = float(textureSize(LutTexture,0).y);
    float maxColor = N - 1.0;

    // index (slice) for the blue channel
    float cell = floor(color.b * maxColor);

    // compute normalized UV that maps the packed (N*N x N) LUT layout:
    // u = (cell*N + color.r*(N-1) + 0.5) / (N*N)
    // v = (color.g*(N-1) + 0.5) / N
    // the +0.5 replicates the half-texel offset used in the HLSL code.
    float u = (cell * N + color.r * maxColor + 0.5) / (N * N);
    float v = (color.g * maxColor + 0.5) / N;

    // D3D/HLSL vs OpenGL texture origin: flip Y if required
    //v = 1.0 - v;

    vec3 gradedCol = textureLod(LutTexture, vec2(u, v),0.0).rgb;
    return gradedCol;
}

void main() 
{

    // Calculate the position within the 4x4 Bayer matrix
    int x = int(gl_FragCoord.x) % 4;
    int y = int(gl_FragCoord.y) % 4;
    int index = y * 4 + x;
    float bayer_value = bayer4x4[index];
    
    float aspectRatio = getAspectRatio();

    ivec2 res = ivec2(screenResolution);
    // Sample the texture color (16-bit precision, normalized to [0,1])
    vec3 color;

    if(true)
    {
        color = applyFxaa(screenTexture, gl_FragCoord.xy, vec2(res)).rgb;
    }
    else
    {
        ivec2 size = textureSize(screenTexture, 0);      // requires GL 3.0 / GLES3
        ivec2 texel = ivec2(floor(TexCoords * vec2(size))); // integer texel coords
        color = texelFetch(screenTexture, texel, 0).rgb;
    }



    color = GetFromLUT(color);

    color = smoothPosterize(color, 70.0,0.35,TexCoords*vec2(aspectRatio,1.0));


    if(false) 
    {
        vec3 colorHue = normalize(color);
        float colorBrightness = length(color);

        colorBrightness = mix(colorBrightness, 0.3f, 0.1f);

        colorHue = mix(colorHue, vec3(1, 0.15f, 0.15f), 0.25f);
        color = colorHue * colorBrightness;
    }

    // Apply dithering by adding the scaled Bayer value to RGB channels
    // Use /256.0 for precise dithering range
    color += bayer_value / 256.0;

    // Output the dithered color
    FragColor = vec4(color,1);
}