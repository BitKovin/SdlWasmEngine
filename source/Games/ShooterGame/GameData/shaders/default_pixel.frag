#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;
in vec3 v_worldPosition;
in vec3 v_light;

out vec4 FragColor;
uniform sampler2D u_texture; 
uniform sampler2D u_textureEmissive; 

uniform vec3 cameraPosition;

uniform bool masked;

uniform bool is_particle;
uniform bool is_decal;

uniform vec3 light_color; 
uniform vec3 direct_light_color; 
uniform vec3 direct_light_dir; 

uniform float fog_start;
uniform float fog_end;
uniform float fog_opacity;
uniform vec3  fog_color;

vec3 CalculateLight();

vec4 ApplyFog(vec4 fragColor);

void main() {
    vec4 texColor;
    

    texColor = texture(u_texture, v_texcoord) * v_color;
    

    vec3 color = texColor.rgb;
    float alpha = texColor.a;


    if(masked)
    {

        if(alpha < 0.5f) 
        {
            discard;
            return;
        }

        alpha = 1.0;

    }



    vec3 ligthColor = CalculateLight() + texture(u_textureEmissive, v_texcoord).rgb;

    color *= ligthColor;

    FragColor = vec4(color, alpha);
}

vec3 CalculateLight()
{

    vec3 normal = normalize(v_normal);
    
    if (!gl_FrontFacing)
        normal = -normal;

    float factor = clamp(dot(normal, normalize(direct_light_dir)) * 0.8 + 0.2, 0.0, 1.0);
    vec3 light = mix(light_color, direct_light_color, factor);

    light *= 3.0f;

    light += v_light;

    return light;
}

vec4 ApplyFog(vec4 fragColor)
{
    float fragDistance = distance(v_worldPosition, cameraPosition);

    // Adjust for negative fog_start
    float start = max(fog_start, 0.0); // optional, to avoid weird behavior if you want
    float end = max(fog_end, 0.0001);  // avoid division by zero

    // Compute fog factor
    float fogFactor = (fragDistance - fog_start) / (fog_end - fog_start);

    // Clamp between 0 and 1
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Apply global fog opacity
    fogFactor *= fog_opacity;

    // Blend scene color with fog color
    return vec4(mix(fragColor.rgb, fog_color, fogFactor), fragColor.a);
}
