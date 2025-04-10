#version 300 es
precision highp float;
in vec2 v_texcoord;
out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict

void main() {
    FragColor = texture(u_texture, v_texcoord);
    //FragColor = vec4(1,1,1,1);
}