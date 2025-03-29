#version 300 es
precision mediump float;
in vec2 v_texcoord;
out vec4 FragColor;
uniform sampler2D texture;

void main() {
    FragColor = texture(texture, v_texcoord);
}