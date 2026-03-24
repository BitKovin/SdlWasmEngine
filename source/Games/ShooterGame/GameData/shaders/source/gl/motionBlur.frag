#version 300 es
precision highp float;
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;   // current frame
uniform sampler2D uAccumulated;    // previous accumulated blur
uniform sampler2D uCustomIdTex;    // previous accumulated blur
uniform float uDeltaTime;          // seconds since last frame
uniform float uPersistence;        // blur persistence (τ) in seconds
uniform float GameTime;            // absolute game time in seconds
uniform float uMotionScale;        // NEW: user-defined motion intensity

// --- cheap hash function for static per-pixel offsets
vec2 hash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)),
             dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453123);
}

int decodeId24(vec3 color) //use in other place to decode
{
    int r = int(color.r * 255.0 + 0.5);
    int g = int(color.g * 255.0 + 0.5);
    int b = int(color.b * 255.0 + 0.5);

    return (r << 16) | (g << 8) | b;
}

void main()
{

    FragColor = vec4(0);
    return;

    int customId = decodeId24(texture(uCustomIdTex, TexCoords).rgb);

    vec4 history = texture(uAccumulated, TexCoords);

    // --- constants
    const float STATIC_JITTER = 0.0025;
    const float LEGACY_BASE_PER_FRAME = 0.0025;
    const float MOTION_VELOCITY_UV_PER_SEC = LEGACY_BASE_PER_FRAME * 60.0; // 0.0025 per frame @ 60fps

    float dt = min(uDeltaTime, 0.1);

    // --- static jitter (time-independent)
    vec2 staticOffset = (hash2(TexCoords) - 0.5) * STATIC_JITTER;

    // --- motion direction from time + spatial variation
    float t = GameTime * 0.8;
    float spatial = sin(TexCoords.x * 6.2831853 * 2.0) * 0.25
                  + cos(TexCoords.y * 6.2831853 * 1.5) * 0.25;
    float angle = t + spatial;
    vec2 dir = normalize(vec2(cos(angle), sin(angle)));

    // --- motion offset, scaled by uDeltaTime and user variable
    float motionMagnitude = MOTION_VELOCITY_UV_PER_SEC * dt * uMotionScale;
    const float MAX_MOTION_UV = 0.05;
    motionMagnitude = min(motionMagnitude, MAX_MOTION_UV);

    vec2 motionOffset = dir * motionMagnitude;

    // --- combine offsets
    vec2 totalOffset = staticOffset + motionOffset;

    // --- sample current frame
    vec4 c_center = texture(screenTexture, TexCoords);
    vec4 c_p = texture(screenTexture, clamp(TexCoords + totalOffset, 0.0, 1.0));
    vec4 c_m = texture(screenTexture, clamp(TexCoords - totalOffset, 0.0, 1.0));
    vec4 currentBlur = (c_center * 0.5) + (c_p + c_m) * 0.25;
    
    if(customId != 1)
    {
        currentBlur.a = 0.0f;
    }

    // --- sample history (stretched only by motion offset)
    vec4 historyOffset = texture(uAccumulated,
                                 clamp(TexCoords - motionOffset * 0.6, 0.0, 1.0));
    vec4 historyCombined = mix(history, historyOffset, 0.5);

    float w;

    // --- exponential accumulation
    if(customId != 1) 
    {
        w = exp(-dt / max(1e-6f, uPersistence));
    } 
    else 
    {
        w = exp(-dt / 0.007);
    }
    vec4 accum = mix(currentBlur, historyCombined, w);

    FragColor = accum;
}
