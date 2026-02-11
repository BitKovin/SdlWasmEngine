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

uniform int PointLightsNumber;
uniform vec4 LightPositions[MAX_POINT_LIGHTS]; // xyz = position, w = inner cone (see note)
uniform vec3 LightColors[MAX_POINT_LIGHTS];
uniform float LightRadiuses[MAX_POINT_LIGHTS];
uniform vec4 LightDirections[MAX_POINT_LIGHTS]; // xyz = direction, w = outer cone (see note)

vec3 CalculateSimplePointLight(int i, vec3 pixelPosition, vec3 normal)
{
    // ensure normal is normalized
    normal = normalize(normal);

    // vector from pixel -> light
    vec3 lightVector = LightPositions[i].xyz - pixelPosition;
    float distanceToLight = length(lightVector);

    // early-out if outside light radius or invalid radius
    if (distanceToLight > LightRadiuses[i] || LightRadiuses[i] <= 0.0)
        return vec3(0.0);

    // normalized direction from pixel to light
    vec3 lightDir = lightVector / distanceToLight; // avoids computing length twice

    // dot between vector from light->pixel and the stored spotlight direction
    // note: -lightDir is from light to pixel (since lightDir is pixel->light)
    float lightDot = dot(-lightDir, normalize(LightDirections[i].xyz));

    float innerCone = LightPositions[i].w;
    float outerCone = LightDirections[i].w;

    // smooth transition between outer and inner cones
    float dirFactor = smoothstep(outerCone, innerCone, lightDot);

    if (dirFactor <= 0.001)
        return vec3(0.0);

    // backface / lighting check (preserve original logic)
    if (dot(normal, lightDir) < 0.0)
        return vec3(0.0);

    // simple radial attenuation (linear)
    float dist = max((LightRadiuses[i] - distanceToLight) / LightRadiuses[i], 0.0);
    float intense = dist; // original used just 'dist' (comment shows alternative)
    intense *= max(dot(normal, lightDir), 0.0);
    intense = max(intense, 0.0);    

    vec3 L = LightColors[i] * intense;

    return L * dirFactor;
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

    for (int i = 0; i < min(MAX_POINT_LIGHTS, PointLightsNumber); i++)
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
