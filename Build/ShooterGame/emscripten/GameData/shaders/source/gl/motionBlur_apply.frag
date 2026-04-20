#version 300 es
precision highp float;
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D blurTexture;

void main()
{
    vec4 screen = texture(screenTexture, TexCoords);
    vec4 blur = texture(blurTexture, TexCoords);
    vec3 finalColor = blur.rgb * blur.a + screen.rgb * (1.0 - blur.a);
    FragColor = vec4(finalColor, 1.0);
}