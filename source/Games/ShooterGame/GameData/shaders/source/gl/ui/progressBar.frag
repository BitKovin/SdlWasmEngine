#version 300 es
precision highp float;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

uniform vec4 bgColor;
uniform sampler2D bgTexture;
uniform float progress;

out vec4 FragColor;

void main()
{
    // Determine if this fragment is in the filled or unfilled region
    bool isFilled = v_TexCoord.x <= progress;

    vec4 outColor;

    if (isFilled) {
        // Foreground: progress fill
        vec4 texColor = texture(u_Texture, v_TexCoord);
        outColor = texColor * u_Color;
    } else {
        // Background: unfilled region
        vec4 bgTex = texture(bgTexture, v_TexCoord);
        outColor = bgTex * bgColor;
    }

    // Premultiplied alpha output
    FragColor = vec4(outColor.rgb * outColor.a, outColor.a);
}