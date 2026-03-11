#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;


out vec4 FragColor;
uniform sampler2D u_texture; 

uniform float customId;

vec3 encodeId24(int id)
{
    // Clamp to 24 bits just in case
    id = clamp(id, 0, 16777215); // 2^24 - 1

    float r = float((id >> 16) & 0xFF) / 255.0;
    float g = float((id >> 8) & 0xFF) / 255.0;
    float b = float(id & 0xFF) / 255.0;

    return vec3(r, g, b);
}

int decodeId24(vec3 color) //use in other place to decode
{
    int r = int(color.r * 255.0 + 0.5);
    int g = int(color.g * 255.0 + 0.5);
    int b = int(color.b * 255.0 + 0.5);

    return (r << 16) | (g << 8) | b;
}

void main() {
    vec4 texColor = texture(u_texture, v_texcoord) * v_color;

    if(texColor.a<0.999)
    {
        discard;
        return;
    }
        

    FragColor = vec4(encodeId24(int(customId)), 1);
}