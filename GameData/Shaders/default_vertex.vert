#version 300 es
precision mediump float;
layout(location = 0) in vec3 Position;
layout(location = 2) in vec2 TextureCoordinate;
out vec2 v_texcoord;

uniform mat4x4 world;
uniform mat4x4 view;
uniform mat4x4 projection;

void main() {
    v_texcoord = TextureCoordinate;
    
    gl_Position = projection * view * world * vec4(Position, 1.0);
}