$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(screenTexture, 0);
SAMPLER2D(uAccumulated,  1);
SAMPLER2D(uCustomIdTex,  2);

uniform vec4 uDeltaTime;    // .x
uniform vec4 uPersistence;  // .x
uniform vec4 GameTime;      // .x
uniform vec4 uMotionScale;  // .x

vec2 hash2(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)),
             dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453123);
}

int decodeId24(vec3 color)
{
    int r = int(color.r * 255.0 + 0.5);
    int g = int(color.g * 255.0 + 0.5);
    int b = int(color.b * 255.0 + 0.5);
    return r * 65536 + g * 256 + b;  // no <<, no |
}

void main()
{
 
    vec2 screenCoords = v_texcoord0;

    #if BGFX_SHADER_LANGUAGE_GLSL
    
    #else

    screenCoords.y = 1.0 - screenCoords.y;

    #endif

    int customId = decodeId24(texture2D(uCustomIdTex, screenCoords).rgb);

    vec4 history = texture2D(uAccumulated, screenCoords);

    const float STATIC_JITTER              = 0.0025;
    const float LEGACY_BASE_PER_FRAME      = 0.0025;
    const float MOTION_VELOCITY_UV_PER_SEC = LEGACY_BASE_PER_FRAME * 60.0;

    float dt = min(uDeltaTime.x, 0.1);

    vec2  staticOffset = (hash2(screenCoords) - 0.5) * STATIC_JITTER;

    float t       = GameTime.x * 0.8;
    float spatial = sin(screenCoords.x * 6.2831853 * 2.0) * 0.25
                  + cos(screenCoords.y * 6.2831853 * 1.5) * 0.25;
    float angle = t + spatial;
    vec2  dir   = normalize(vec2(cos(angle), sin(angle)));

    float motionMagnitude = MOTION_VELOCITY_UV_PER_SEC * dt * uMotionScale.x;
    const float MAX_MOTION_UV = 0.05;
    motionMagnitude = min(motionMagnitude, MAX_MOTION_UV);

    vec2 motionOffset = dir * motionMagnitude;
    vec2 totalOffset  = staticOffset + motionOffset;

    vec4 c_center  = texture2D(screenTexture, screenCoords);
    vec4 c_p       = texture2D(screenTexture, clamp(screenCoords + totalOffset, 0.0, 1.0));
    vec4 c_m       = texture2D(screenTexture, clamp(screenCoords - totalOffset, 0.0, 1.0));
    vec4 currentBlur = (c_center * 0.5) + (c_p + c_m) * 0.25;

    if (customId != 1)
        currentBlur.a = 0.0;

    vec4 historyOffset  = texture2D(uAccumulated, clamp(screenCoords - motionOffset * 0.6, 0.0, 1.0));
    vec4 historyCombined = mix(history, historyOffset, 0.5);

    float w;
    if (customId != 1)
        w = exp(-dt / max(1e-6, uPersistence.x));
    else
        w = exp(-dt / 0.007);

    gl_FragColor = mix(currentBlur, historyCombined, w);
}
