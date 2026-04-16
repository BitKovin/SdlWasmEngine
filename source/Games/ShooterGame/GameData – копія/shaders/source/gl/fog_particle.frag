#version 300 es
precision highp float;

in vec2 v_texcoord;
in vec4 v_color;

out vec4 FragColor;

uniform sampler2D u_texture;      // particle texture
uniform sampler2D depthTexture;   // resolved scene depth (non-MSAA)
uniform vec2  u_invViewport;      // 1 / (width, height)
uniform float u_near;
uniform float u_far;
uniform float u_softDistance;     // e.g. 1.0..3.0 (world units if linearized)

float LinearizeDepth(float d) {
    float z = d * 2.0 - 1.0; // [0,1] -> NDC [-1,1]
    return (2.0 * u_near * u_far) / (u_far + u_near - z * (u_far - u_near));
}

void main() {
    vec4 texColor = texture(u_texture, v_texcoord) * v_color;
    if (texColor.a < 0.001) discard;

    // Sample scene depth at this pixel
    vec2 screenUV   = gl_FragCoord.xy * u_invViewport;
    float sceneNDC  = texture(depthTexture, screenUV).r;   // [0,1]
    float fragNDC   = gl_FragCoord.z;                      // [0,1]

    // --- Option A (simple, works well) ---
    // If your main pass uses reversed-Z, flip the sign: (fragNDC - sceneNDC)
    //float fade = clamp((sceneNDC - fragNDC) * (u_softDistance * 200.0), 0.0, 1.0);

    // --- Option B (world-unit fade; comment Option A and use this) ---
// Try normal Z
float diff = sceneNDC - fragNDC;

// If everything is invisible, flip the sign:
diff = fragNDC - sceneNDC;

// Normalize by a soft range in NDC (try 0.01 first)
float fade = clamp(diff / 0.04, 0.0, 1.0);


    FragColor = vec4(texColor.rgb, texColor.a * fade);
}
