$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(screenTexture, 0);
SAMPLER2D(blurTexture,   1);

void main()
{

    vec2 screenCoords = v_texcoord0;
    vec2 blurCoords = v_texcoord0;

    #if BGFX_SHADER_LANGUAGE_GLSL

    #else
    screenCoords.y = 1.0 - screenCoords.y;
    blurCoords.y = 1.0 - blurCoords.y;
    #endif

    vec4 screen = texture2D(screenTexture, screenCoords);
    vec4 blur   = texture2D(blurTexture,   blurCoords);
    
    vec3 finalColor = blur.rgb * blur.a + screen.rgb * (1.0 - blur.a);
    gl_FragColor = vec4(finalColor, 1.0);
}
