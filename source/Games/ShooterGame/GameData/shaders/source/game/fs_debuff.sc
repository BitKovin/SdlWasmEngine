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

    // Angle (start at top, clockwise).
    // atan2 is used instead of GLSL two-arg atan() for bgfx cross-backend compat.
    float angle = atan2(-p.x, p.y);
    angle = (angle + 3.14159265) / (2.0 * 3.14159265);

    // Progress mask
    float progressMask = step(angle, progress);

    return ring * progressMask;
}

void main()
{
    vec2 iconUV = mix(v_texcoord0, vec2(0.5, 0.5), -0.1);
    vec2 bgUV   = mix(v_texcoord0, vec2(0.5, 0.5), -0.1);

    // Clamp out-of-range UVs to vec2(0) so the texture fetch returns transparent
    if (any(lessThan(iconUV, vec2_splat(0.0))) || any(greaterThan(iconUV, vec2_splat(1.0))))
        iconUV = vec2(0.0, 0.0);

    if (any(lessThan(bgUV, vec2_splat(0.0))) || any(greaterThan(bgUV, vec2_splat(1.0))))
        bgUV = vec2(0.0, 0.0);

    vec4 texColor = vec4_splat(0.0);
    if (iconUV.x != 0.0 || iconUV.y != 0.0)
        texColor = texture2D(u_Texture, iconUV);

    vec4 circleColor = vec4_splat(0.0);
    if (bgUV.x != 0.0 || bgUV.y != 0.0)
        circleColor = texture2D(circleTexture, bgUV) * u_Color;

    float fillProgress = step(1.0 - u_progress.x, v_texcoord0.y);

    // Vertical fill: dim when below progress threshold, full u_Color above
    circleColor *= mix(vec4(0.07, 0.07, 0.07, 0.55), u_Color, fillProgress);

    vec4 baseColor = mix(circleColor, texColor, texColor.a);
    baseColor.a = circleColor.a;

    /* ==============================
       Circular outline
       ============================== */
    float ringAlpha = CircularProgressRing(
        v_texcoord0,
        vec2(0.5, 0.5),
        0.4,
        0.499,
        u_timeRel.x
    );

    vec4 ringColor = vec4(0.949, 0.925, 0.922, ringAlpha);

    vec4 outColor = mix(baseColor, ringColor, ringColor.a);

    if (outColor.a < 0.001)
        discard;

    // Premultiplied alpha output — matches the engine's Blend::Premultiplied state
    gl_FragColor = vec4(outColor.rgb * outColor.a, outColor.a);
}
