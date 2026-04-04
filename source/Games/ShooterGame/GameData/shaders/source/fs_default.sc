$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1

#include <bgfx_shader.sh>

SAMPLER2D(u_texture,         0);
SAMPLER2D(u_textureEmissive, 1);

uniform vec4 cameraPosition;

uniform vec4 masked;

uniform vec4 flipedNormals;

uniform vec4 light_color;
uniform vec4 direct_light_color;
uniform vec4 direct_light_dir;

uniform vec4 fog_start;
uniform vec4 fog_end;
uniform vec4 fog_opacity;
uniform vec4 fog_color;

uniform vec4 modelColor;

uniform vec4 PointLightsNumber;

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 16
#endif

uniform mat4 PointLights[MAX_POINT_LIGHTS];

vec3 CalculateSimplePointLight(int i, vec3 pixelPosition, vec3 normal)
{
    // === PORTABLE MATRIX ACCESS (bgfx built-in) ===
    // mtxGetColumn works correctly on EVERY backend:
    //   • OpenGL/GLSL  → column-major (exactly like your original [0]/[1]/[2])
    //   • DirectX/HLSL, Metal, Vulkan/SPIR-V → row-major storage
    // It automatically extracts the logical column no matter how the backend stores the matrix.
    vec4 col0 = mtxGetColumn(PointLights[i], 0);
    vec4 col1 = mtxGetColumn(PointLights[i], 1);
    vec4 col2 = mtxGetColumn(PointLights[i], 2);

    vec3 lightPos   = col0.xyz;
    float innerCone = col0.w;
    vec3 lightCol   = col1.rgb;
    float radius    = col1.w;
    vec3 lightFwd   = col2.xyz;
    float outerCone = col2.w;

    if (radius <= 0.0) return vec3(0.0, 0.0, 0.0);

    normal = normalize(normal);
    vec3 lightVector = lightPos - pixelPosition;
    float distanceToLight = length(lightVector);
    if (distanceToLight > radius) return vec3(0.0, 0.0, 0.0);

    vec3 lightDir = lightVector / distanceToLight;
    float lightDot = dot(-lightDir, normalize(lightFwd));
    float dirFactor = smoothstep(outerCone, innerCone, lightDot);
    if (dirFactor <= 0.001) return vec3(0.0, 0.0, 0.0);

    if (dot(normal, lightDir) < 0.0) return vec3(0.0, 0.0, 0.0);

    float dist = max((radius - distanceToLight) / radius, 0.0);
    float intense = dist * max(dot(normal, lightDir), 0.0);

    return lightCol * intense * dirFactor;
}

// Varyings are only in scope in main() — pass them in as parameters
vec3 CalculateLight(vec3 normal, vec3 worldPosition)
{
    normal = normalize(normal);


    float factor = clamp(dot(normal, normalize(direct_light_dir.xyz)) * 0.8 + 0.2, 0.0, 1.0);
    vec3 light   = mix(light_color.rgb, direct_light_color.rgb, factor);

    light *= 3.0;

    int numLights = min(MAX_POINT_LIGHTS, int(PointLightsNumber.x));
    for (int i = 0; i < numLights; i++)
        light += CalculateSimplePointLight(i, worldPosition, normal);

    return light;
}

vec4 ApplyFog(vec4 fragColor, vec3 worldPosition)
{
    float fragDistance = distance(worldPosition, cameraPosition.xyz);
    float fogFactor    = (fragDistance - fog_start.x) / (fog_end.x - fog_start.x);
    fogFactor          = clamp(fogFactor, 0.0, 1.0) * fog_opacity.x;
    return vec4(mix(fragColor.rgb, fog_color.rgb, fogFactor), fragColor.a);
}

void main()
{
    vec4  texColor = texture2D(u_texture, v_texcoord0) * v_color0 * modelColor;
    vec3  color    = texColor.rgb;
    float alpha    = texColor.a;

    if (masked.x > 0.0)
    {
        if (alpha < 0.5)
            discard;
        alpha = 1.0;
    }

    vec3 normal = v_normal;

    if (flipedNormals.x > 0.0)
        normal = -normal;

    vec3 lightColor = CalculateLight(normal, v_world.xyz)
                    + texture2D(u_textureEmissive, v_texcoord0).rgb;
    color *= lightColor;

    gl_FragColor = ApplyFog(vec4(color, alpha), v_world.xyz);
}
