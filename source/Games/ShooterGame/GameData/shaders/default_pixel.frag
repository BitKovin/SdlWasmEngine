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

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 16
#endif

uniform float PointLightsNumber;
uniform vec4 LightPositions[MAX_POINT_LIGHTS]; // xyz = position, w = inner cone (see note)
uniform vec3 LightColors[MAX_POINT_LIGHTS];
uniform float LightRadiuses[MAX_POINT_LIGHTS];
uniform vec4 LightDirections[MAX_POINT_LIGHTS]; // xyz = direction, w = outer cone (see note)

uniform mat4 PointLights[MAX_POINT_LIGHTS];

vec3 CalculateSimplePointLight(int i, vec3 pixelPosition, vec3 normal)
{
    // unpack columns
    vec4  col0      = PointLights[i][0];
    vec4  col1      = PointLights[i][1];
    vec4  col2      = PointLights[i][2];

    vec3  lightPos  = col0.xyz;
    float innerCone = col0.w;

    vec3  lightCol  = col1.rgb;
    float radius    = col1.w;

    vec3  lightFwd  = col2.xyz;
    float outerCone = col2.w;

    // early-out on bad radius
    if (radius <= 0.0) return vec3(0.0);

    normal = normalize(normal);

    vec3  lightVector    = lightPos - pixelPosition;
    float distanceToLight = length(lightVector);

    if (distanceToLight > radius) return vec3(0.0);

    vec3  lightDir = lightVector / distanceToLight;

    float lightDot  = dot(-lightDir, normalize(lightFwd));
    float dirFactor = smoothstep(outerCone, innerCone, lightDot);

    if (dirFactor <= 0.001)              return vec3(0.0);
    if (dot(normal, lightDir) < 0.0)    return vec3(0.0);

    float dist    = max((radius - distanceToLight) / radius, 0.0);
    float intense = dist * max(dot(normal, lightDir), 0.0);

    return lightCol * intense * dirFactor;
}

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

    for (int i = 0; i < min(MAX_POINT_LIGHTS, int(PointLightsNumber)); i++)
	{
		light += CalculateSimplePointLight(i, v_worldPosition, v_normal);
	}

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
