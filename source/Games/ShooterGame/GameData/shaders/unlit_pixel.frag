#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;

out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict



void main() {
    vec4 texColor = texture(u_texture, v_texcoord) *  v_color;

    vec3 color = texColor.rgb;
    float alpha = texColor.a;

    if(alpha <0.01)
        discard;


    FragColor = vec4(color, alpha);
}

