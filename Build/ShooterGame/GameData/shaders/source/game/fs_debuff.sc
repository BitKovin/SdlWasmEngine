$input v_texcoord0

#include <../bgfx_shader.sh>

SAMPLER2D(u_Texture,     0);
SAMPLER2D(circleTexture, 1);  // @texture GameData/textures/ui/circle.png

uniform vec4 u_Color;

// float uniforms packed into vec4 — access .x.  Names kept identical.
uniform vec4 u_progress;
uniform vec4 u_timeRel;  // circular progress [0..1]

/* ==========================================
   Circular progress ring function
   ========================================== */
float CircularProgressRing(
    vec2  uv,
    vec2  center,
    float innerRadius,
    float outerRadius,
    float progress
)
{
    vec2  p    = uv - center;
    float dist = length(p);

    // Ring mask
    float ring = step(innerRadius, dist) * step(dist, outerRadius);

    // Angle (start at top, clockwise)
    float angle = atan2(p.y, p.x);         // atan2 standard
    angle = (0.25 - angle / (2.0 * 3.14159265)); // shift 0 = top
    angle = fract(angle);                  // wrap 0..1

    // Progress mask
    float progressMask = step(angle, progress);

    return ring * progressMask;
}

void main()
{

    vec2 coord = v_texcoord0;

    #if BGFX_SHADER_LANGUAGE_GLSL

        coord.y = 1.0 - coord.y;
    #else
        coord.y = 1.0 - coord.y;
    #endif

    // Slight UV offset can be negative; clamp instead of snapping to zero
    vec2 iconUV = mix(coord, vec2(0.5, 0.5), -0.1);
    vec2 bgUV   = mix(coord, vec2(0.5, 0.5), -0.1);

    iconUV = clamp(iconUV, 0.0, 1.0);
    bgUV   = clamp(bgUV,   0.0, 1.0);


    #if BGFX_SHADER_LANGUAGE_GLSL

    vec4 texColor    = texture2D(u_Texture, vec2(0.0, 1.0) - iconUV);
    vec4 circleColor = texture2D(circleTexture, vec2(0.0, 1.0) - bgUV) * u_Color;

    #else
    vec4 texColor    = texture2D(u_Texture,vec2(0.0, 1.0) - iconUV);
    vec4 circleColor = texture2D(circleTexture, vec2(0.0, 1.0) - bgUV) * u_Color;  
    #endif

    // Sample textures


    // Vertical fill: top-to-bottom
    float fillProgress = step(coord.y, u_progress.x); 
    circleColor *= mix(vec4(0.07, 0.07, 0.07, 0.55), u_Color, fillProgress);

    // Base color blend (icon over circle)
    vec4 baseColor = mix(circleColor, texColor, texColor.a);
    baseColor.a = circleColor.a;

    /* ==============================
       Circular outline
       ============================== */
    float ringAlpha = CircularProgressRing(
        coord,
        vec2(0.5, 0.5),
        0.4,
        0.499,
        u_timeRel.x
    );

    vec4 ringColor = vec4(0.949, 0.925, 0.922, ringAlpha);
    vec4 outColor  = mix(baseColor, ringColor, ringColor.a);

    // Discard transparent pixels
    if (outColor.a < 0.001)
        discard;

    // Premultiplied alpha output
    gl_FragColor = vec4(outColor.rgb * outColor.a, outColor.a);
}
