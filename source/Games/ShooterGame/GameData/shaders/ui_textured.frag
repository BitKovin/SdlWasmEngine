#version 300 es
precision highp float;


in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    vec4 outColor = texColor * u_Color;
    FragColor = vec4(outColor.rgb * outColor.a, outColor.a);
}
