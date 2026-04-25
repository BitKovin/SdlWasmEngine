$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

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

uniform vec4 rim_pow; // @ (4.0, 0.0, 0.0, 0.0) - Controls rim thickness
uniform vec4 rim_color; // @ (1.0, 1.0, 0.2, 1.0) - Rim color
uniform vec4 specular_pow; // @ (5.0, 0.0, 0.0, 0.0) - Controls specular sharpness
uniform vec4 specular_scale; // @ (0.05, 0.0, 0.0, 0.0) - Scales specular intensity

uniform vec4 modelColor;
uniform vec4 PointLightsNumber;

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 16
#endif

uniform mat4 PointLights[MAX_POINT_LIGHTS];

// ==========================================
// 1. STYLED LIGHTING HELPER FUNCTIONS (DEADLOCK STYLE)
// ==========================================

float ComputeStyledDiffuse(vec3 normal, vec3 lightDir)

{

    float factor = clamp(dot(normal, normalize(lightDir)) * 0.7 + 0.3, 0.0, 1.0);



    const float shadow_start = 0.1;

    const float shadow_end = 0.4;

    const float mid_start = 0.4;

    const float mid_end = 0.9;

    const float curve_mix = 0.5;



    float low_curve = smoothstep(shadow_start, shadow_end, factor);

    float high_curve = smoothstep(mid_start, mid_end, factor);


    return mix(mix(low_curve, high_curve, curve_mix),factor, 0.5);

}

float ComputeStyledSpecular(vec3 normal, vec3 lightDir, vec3 viewDir)
{
    vec3 half_vector = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, half_vector), 0.0);

    // Standard specular power
    float specular_raw = pow(NdotH, specular_pow.x); 

    // Deadlock NPR Specular Controls
    const float steps = 2.0;       // Number of specular rings
    const float smoothness = 0.1; // Controls the softness of the blend. 
                                   // 0.01 = hard snap, 0.5 = completely smooth/linear

    // Scale the raw specular up to the number of steps
    float scaled_spec = specular_raw * steps;
    
    // Separate into the base integer step and the fractional remainder
    float base_step = floor(scaled_spec);
    float fraction = fract(scaled_spec);

    // Smooth out the fractional part to create a "terraced" transition
    float smoothed_fraction = smoothstep(0.5 - smoothness, 0.5 + smoothness, fraction);

    // Recombine and scale back down to a 0.0 - 1.0 range
    float specular_final = clamp((base_step + smoothed_fraction) / steps, 0.0, 1.0);

    // Specular should only appear on the lit side of the model
    float NdotL = max(dot(normal, lightDir), 0.0);
    float litMask = smoothstep(0.0, 0.1, NdotL); 
    
    return specular_final * litMask * specular_scale.x;
}

vec3 CalculateContourRim(vec3 normal, vec3 viewDir)
{
    float NdotV = max(dot(normal, viewDir), 0.0);
    float fresnel = 1.0 - NdotV;
    
    // Upward Ramp Bias
    float upBias = max(0.0, dot(normal, vec3(0.0, 1.0, 0.0))) * 0.7 + 0.3;

    // Base rim calculation
    float rimEdge = pow(fresnel, rim_pow.x) * upBias;
    
    // 1. Tighter Threshold: Push this higher so it demands a steeper silhouette edge.
    // (Increased from 0.4/0.45 to 0.5/0.6. You may need to tune this based on your rim_pow.x)
    rimEdge = smoothstep(0.45, 0.55, rimEdge);
    
    // 2. Strict Front-Face Kill:
    // If the surface is facing the camera (NdotV is high), force the rim light to 0.
    // This transitions from 1.0 at NdotV=0.2 (grazing) down to 0.0 at NdotV=0.4 (facing).
    float frontMask = smoothstep(0.4, 0.2, NdotV); 
    
    return rimEdge * frontMask * rim_color.rgb * 0.1; 
}

// ==========================================
// 2. LIGHT ACCUMULATION
// ==========================================

void AccumulatePointLight(int i, vec3 pixelPosition, vec3 normal, vec3 viewDir, inout vec3 outDiffuse, inout vec3 outSpecular)
{
    vec4 col0 = mtxGetColumn(PointLights[i], 0);
    vec4 col1 = mtxGetColumn(PointLights[i], 1);
    vec4 col2 = mtxGetColumn(PointLights[i], 2);

    vec3 lightPos   = col0.xyz;
    float innerCone = col0.w;
    vec3 lightCol   = col1.rgb;
    float radius    = col1.w;
    vec3 lightFwd   = col2.xyz;
    float outerCone = col2.w;

    if (radius <= 0.0) return;

    vec3 lightVector = lightPos - pixelPosition;
    float distanceToLight = length(lightVector);
    if (distanceToLight > radius) return;

    vec3 lightDir = lightVector / distanceToLight;
    float lightDot = dot(-lightDir, normalize(lightFwd));
    float dirFactor = smoothstep(outerCone, innerCone, lightDot);
    if (dirFactor <= 0.001) return;

    float distAtten = max((radius - distanceToLight) / radius, 0.0);
    float intensity = distAtten * dirFactor;

    float diffFactor = ComputeStyledDiffuse(normal, lightDir);
    float specFactor = ComputeStyledSpecular(normal, lightDir, viewDir);

    outDiffuse  += lightCol * diffFactor * intensity;
    outSpecular += lightCol * specFactor * intensity;
}

void CalculatePointLights(vec3 normal, vec3 worldPosition, vec3 viewDir, inout vec3 totalDiffuse, inout vec3 totalSpecular)
{
    int numLights = min(MAX_POINT_LIGHTS, int(PointLightsNumber.x));
    for (int i = 0; i < numLights; i++)
    {
        AccumulatePointLight(i, worldPosition, normal, viewDir, totalDiffuse, totalSpecular);
    }
}

vec3 CalculateDirectionalDiffuse(vec3 normal, vec3 lightDir)
{
    float diffuse_factor = ComputeStyledDiffuse(normal, lightDir);
    
    // Deadlock anchors exposure to a target value. We do this by setting a strong ambient floor
    // combined with the heavily weighted directional light.
    vec3 ambient = light_color.rgb * 0.7f; // Base ambient
    vec3 diffuse = direct_light_color.rgb;

    return mix(ambient, diffuse, diffuse_factor) * 1.0;
}

vec3 CalculateDirectionalSpecular(vec3 normal, vec3 lightDir, vec3 viewDir)
{
    float specFactor = ComputeStyledSpecular(normal, lightDir, viewDir);
    return direct_light_color.rgb * specFactor;
}

vec4 ApplyFog(vec4 fragColor, vec3 worldPosition)
{
    float fragDistance = distance(worldPosition, cameraPosition.xyz);
    float fogFactor    = (fragDistance - fog_start.x) / (fog_end.x - fog_start.x);
    fogFactor          = clamp(fogFactor, 0.0, 1.0) * fog_opacity.x;
    return vec4(mix(fragColor.rgb, fog_color.rgb, fogFactor), fragColor.a);
}

// ==========================================
// 3. MAIN
// ==========================================

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

    vec3 N = normalize(v_normal);
    vec3 T = normalize(v_tangent);
    vec3 B = normalize(v_bitangent);

    if (flipedNormals.x > 0.5)
    {
        N = -N; T = -T; B = -B;
    }

    T = normalize(T - N * dot(N, T));
    float handedness = sign(dot(cross(N, T), B));
    B = normalize(cross(N, T) * handedness);

    mat3 TBN = mtxFromCols(T, B, N);

    vec3 mapN = vec3(0.5, 0.5, 1.0); // Assume flat normal map for default
    mapN = mapN * 2.0 - 1.0;
    mapN = normalize(mapN);

    vec3 normal = normalize(mul(TBN, mapN));

    if(length(T) < 0.1)
        normal = N;

    vec3 lightDir = normalize(direct_light_dir.xyz);
    vec3 viewDir  = normalize(cameraPosition.xyz - v_world.xyz);

    // Accumulate Lighting
    vec3 dirDiffuse  = CalculateDirectionalDiffuse(normal, lightDir);
    vec3 dirSpecular = CalculateDirectionalSpecular(normal, lightDir, viewDir);
    
    vec3 pointDiffuse  = vec3(0.0,0.0,0.0);
    vec3 pointSpecular = vec3(0.0,0.0,0.0);
    CalculatePointLights(normal, v_world.xyz, viewDir, pointDiffuse, pointSpecular);

    vec3 emissive = texture2D(u_textureEmissive, v_texcoord0).rgb;

    // View-dependent rim light
    vec3 rim = vec3(0,0,0);
    //rim = CalculateContourRim(normal, viewDir);

    // Composition (Light Weight Mixing)
    vec3 finalColor =
        color * (dirDiffuse + pointDiffuse) // Albedo modulated by stepped diffuse
        + dirSpecular + pointSpecular       // Additive quantized specular
        + rim                               // Additive camera-attached rim
        + emissive;                         // Self-illumination


    gl_FragColor = ApplyFog(vec4(finalColor, alpha), v_world.xyz);
}