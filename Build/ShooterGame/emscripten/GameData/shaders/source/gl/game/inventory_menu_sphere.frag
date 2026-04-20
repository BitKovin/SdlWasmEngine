#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;

out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict

uniform vec3 cameraPosition;

uniform bool is_particle;
uniform bool is_decal;

uniform vec3 light_color; 
uniform vec3 direct_light_color; 
uniform vec3 direct_light_dir; 



void main() {

    vec3 color = vec3(0.17f, 0.09f, 0.09f); // Default to white color
    float alpha = 0.75; // Default to fully opaque

    FragColor = vec4(color, alpha);
}

