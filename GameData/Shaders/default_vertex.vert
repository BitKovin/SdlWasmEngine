#version 300 es
precision mediump float;
layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texcoord;
out vec2 v_texcoord;

void main() {
    v_texcoord = texcoord;
    gl_Position = vec4(position.xy, 0.0, 1.0);
}