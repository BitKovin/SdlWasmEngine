#version 300 es
precision highp float;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform sampler2D circleTexture;// @texture GameData/textures/ui/circle.png
uniform vec4 u_Color;

uniform float u_progress;
uniform float u_timeRel; // circular progress [0..1]

out vec4 FragColor;

/* ==========================================
   Circular progress ring function
   ========================================== */
float CircularProgressRing(
    vec2 uv,
    vec2 center,
    float innerRadius,
    float outerRadius,
    float progress
) {
    vec2 p = uv - center;
    float dist = length(p);

    // Ring mask
    float ring = step(innerRadius, dist) * step(dist, outerRadius);

    // Angle (start at top, clockwise)
    float angle = atan(-p.x, p.y);
    angle = (angle + 3.14159265) / (2.0 * 3.14159265);

    // Progress mask
    float progressMask = step(angle, progress);

    return ring * progressMask;
}

void main()
{

    vec2 iconUV = mix(v_TexCoord,vec2(0.5), -0.1);
    vec2 bgUV = mix(v_TexCoord,vec2(0.5), -0.1);

    // If any component is out of [0,1], set the whole UV to vec2(0)
    if (any(lessThan(iconUV, vec2(0.0))) || any(greaterThan(iconUV, vec2(1.0)))) {
        iconUV = vec2(0.0);
    }

    if (any(lessThan(bgUV, vec2(0.0))) || any(greaterThan(bgUV, vec2(1.0)))) {
        bgUV = vec2(0.0);
    }

    vec4 texColor = vec4(0);
    if(iconUV != vec2(0.0))
        texColor = texture(u_Texture, iconUV);

    vec4 circleColor = vec4(0);

    if(bgUV != vec2(0.0))
        circleColor = texture(circleTexture, bgUV) * u_Color;

    vec4 bgColor = circleColor;

    float fillProgress = step(1.0 - u_progress, v_TexCoord.y);

    // Existing vertical fill logic
    circleColor *= mix(vec4(0.07, 0.07, 0.07, 0.55), u_Color, fillProgress);


    vec4 baseColor = mix(circleColor, texColor, texColor.a);
    baseColor.a = circleColor.a;
    

    /* ==============================
       Circular outline usage
       ============================== */

    float ringAlpha = CircularProgressRing(
        v_TexCoord,
        vec2(0.5),
        0.4,
        0.499,
        u_timeRel
    );

    vec4 ringColor = vec4(0.949, 0.925, 0.922, ringAlpha);

    FragColor = mix(baseColor, ringColor, ringColor.a);
}
