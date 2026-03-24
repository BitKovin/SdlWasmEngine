$input v_texcoord0

#include <../bgfx_shader.sh>

SAMPLER2D(u_Texture,  0);
SAMPLER2D(bgTexture,  1);

uniform vec4 u_Color;
uniform vec4 bgColor;
uniform vec4 progress; // .x = progress value [0..1]

void main()
{
    bool isFilled = v_texcoord0.x <= progress.x;

    vec4 outColor;

    if (isFilled)
    {
        // Foreground: progress fill
        vec4 texColor = texture2D(u_Texture, v_texcoord0);
        outColor = texColor * u_Color;
    }
    else
    {
        // Background: unfilled region
        vec4 bgTex = texture2D(bgTexture, v_texcoord0);
        outColor = bgTex * bgColor;
    }

    // Premultiplied alpha output
    gl_FragColor = vec4(outColor.rgb * outColor.a, outColor.a);
}