$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(screenTexture, 0);
SAMPLER2D(blurTexture,   1);

void main()
{
    vec4 screen = texture2D(screenTexture, v_texcoord0);
    vec4 blur   = texture2D(blurTexture,   v_texcoord0);
    vec3 finalColor = blur.rgb * blur.a + screen.rgb * (1.0 - blur.a);
    gl_FragColor = vec4(finalColor, 1.0);
}
