#version 300 es
precision mediump float;


in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    FragColor = texColor * u_Color;
}
