#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;
in vec3 v_worldPosition;
in vec4 v_clipPosition;

in vec4 v_shadowCoords1;
in vec4 v_shadowCoords2;
in vec4 v_shadowCoords3;
in vec4 v_shadowCoords4;

out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict

uniform vec3 cameraPosition;

uniform highp sampler2DShadow shadowMap;
uniform highp sampler2DShadow shadowMapDetail;
uniform int shadowMapSize;  // width, height of shadow map
uniform vec3 lightDirection;   
uniform float shadowDistance1;
uniform float shadowDistance2;
uniform float shadowDistance3;
uniform float shadowDistance4;
uniform float shadowRadius1;
uniform float shadowRadius2;
uniform float shadowRadius3;
uniform float shadowRadius4;
uniform float shaddowOffsetScale;

uniform bool is_particle;



vec3 CalculateDirectionalLight();

void main() {
    vec4 texColor = texture(u_texture, v_texcoord)*v_color;

    vec3 color = texColor.rgb;
    float alpha = texColor.a;

    if(alpha <0.01)
        discard;

    vec3 ligthColor = CalculateDirectionalLight();

    color*=ligthColor;

    FragColor = vec4(color, alpha);
}

vec2 GetDirectionalShadow(
    vec2  mapOffset,
    float shadowDistance, //radius
    vec4  shadowCoords,
    int   pcfRadius,
    float biasScale
) {
    // 1) Project into [0,1] clip space
    vec4 sc = shadowCoords / shadowCoords.w;
    vec3 pc = sc.xyz * 0.5 + 0.5;

    // 2) Atlas-cell remap (half-scale)
    pc *= vec3(0.5, 0.5, 1.0);
    pc += vec3(mapOffset * vec2(0.5), 0.0);



// --- before your PCF loop ---
    
//  • world units-per-texel (full 2×distance frustum):
float worldTexelSize = (2.0 * shadowDistance) / float(shadowMapSize);

//  • base bias in world units:
float biasWorldBase  = biasScale * worldTexelSize;

//  • bump bias on grazing angles:
float NdotL         = clamp(dot(normalize(v_normal),
                                normalize(lightDirection)),
                           0.0, 1.0);
float biasWorld     = biasWorldBase * (1.0 + (1.0 - NdotL));

//  • normalized depth-bias [0..1]:
float bias          = biasWorld / (shadowDistance + 1000.0) * (float(pcfRadius)+1.0) * 1.0;

//  • PCF radius in UV:
vec2 texelSize      = 0.5 / vec2(shadowMapSize);
vec2 radiusUV       = texelSize * float(pcfRadius);

// --- now your PCF loop as before ---


    // 5) PCF loop
    float sumDirect = 0.0;

    float n = 0.0;
    for (int xo = -pcfRadius; xo <= pcfRadius; ++xo) {
        for (int yo = -pcfRadius; yo <= pcfRadius; ++yo) {
            n++;
            vec2 offs = vec2(xo, yo) * texelSize;
            sumDirect += texture(
                shadowMap,
                vec3(pc.xy + offs,
                     pc.z - bias)
            );

        }
    }

    float directFactor = sumDirect/n;

    float sumIndirect = 0.0;
    float sampleScale = mix(4.0,1.0, directFactor);
    n=0.0;

    for (int xo = -pcfRadius; xo <= pcfRadius; ++xo) 
    {
        for (int yo = -pcfRadius; yo <= pcfRadius; ++yo) 
        {
            n++;
            vec2 offs = vec2(xo, yo) * texelSize * sampleScale;
            sumIndirect += texture(
                shadowMapDetail,
                vec3(pc.xy + offs,
                     pc.z - bias * sampleScale)
            );
        }
    }

    float indirectFactor = sumIndirect/n;

    float directFactorNotDetail = directFactor;

    directFactor = mix(0.0,directFactor, indirectFactor);

    indirectFactor = mix(indirectFactor/2.0 + 0.5,1.0,directFactorNotDetail);

    return vec2(directFactor, indirectFactor);
}

vec3 CalculateDirectionalLight()
{
    // 1) Basic N·L
    float directionPower = is_particle
        ? 1.0
        : clamp(dot(-v_normal, lightDirection), 0.0, 1.0);

    float indirectPower = dot(-v_normal, vec3(0, -1, 0)) / 2.0f + 0.5f;

    if(is_particle)
    {
        indirectPower = 1.0;
    }

    
    indirectPower = mix(0.2,0.35,indirectPower);

    // 2) Distance from the camera
    float dist = v_clipPosition.z;

    
    float biasScales[4]        = float[]( 1.0,
                                      1.0,
                                      1.0,
                                      1.0 );


    // Cascade parameters (unchanged)
    float cascadeDist[4]   = float[](shadowDistance1, shadowDistance2, shadowDistance3, shadowDistance4);
    float cascadeRadius[4] = float[](shadowRadius1,   shadowRadius2,   shadowRadius3,   shadowRadius4);
    vec2  cascadeOff[4]    = vec2[](vec2(0,0), vec2(1,0), vec2(0,1), vec2(1,1));
    vec4  cascadeCoords[4] = vec4[](v_shadowCoords1, v_shadowCoords2, v_shadowCoords3, v_shadowCoords4);
    int   cascadePCF[4]    = int[](2, 1, 1, 1);

    vec2 shadow = vec2(1.0);
    float blendPercent = 0.15; // 15% overlap for all cascades

    if(dist < cascadeDist[3]) {
        bool blended = false;

        // Blend between cascades 0–1, 1–2, 2–3
        for (int i = 0; i < 3; ++i) {
            float prevDist   = (i == 0) ? 0.0 : cascadeDist[i-1];
            float depthSpan  = cascadeDist[i] - prevDist;
            float fadeSize   = depthSpan * blendPercent;
            float fadeStart  = cascadeDist[i] - fadeSize;
            float fadeEnd    = cascadeDist[i];

            if (dist >= fadeStart && dist <= fadeEnd) {
                float blendF = smoothstep(fadeStart, fadeEnd, dist);

                // Sample both cascades using their correct radii
                vec2 sh0 = GetDirectionalShadow(
                    cascadeOff[i],
                    cascadeRadius[i],
                    cascadeCoords[i],
                    cascadePCF[i],
                    biasScales[i]
                );
                vec2 sh1 = GetDirectionalShadow(
                    cascadeOff[i+1],
                    cascadeRadius[i+1],
                    cascadeCoords[i+1],
                    cascadePCF[i+1],
                    biasScales[i+1]
                );
                shadow = mix(sh0, sh1, blendF);
                blended = true;
                break;
            }
        }

        // If not in a fade region, pick the matching cascade
        if (!blended) {
            int idx = 0;
            for (int i = 0; i < 4; ++i) {
                if (dist <= cascadeDist[i]) {
                    idx = i;
                    break;
                }
            }
            shadow = GetDirectionalShadow(
                cascadeOff[idx],
                cascadeRadius[idx],
                cascadeCoords[idx],
                cascadePCF[idx],
                biasScales[idx]
            );
        }
    }

    

    directionPower *= shadow.x;
    indirectPower *= shadow.y;

    return mix(vec3(indirectPower), vec3(1.0), directionPower);
}