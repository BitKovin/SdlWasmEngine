$input v_texcoord0
#include <bgfx_shader.sh>
SAMPLER2D(u_Texture, 0);
uniform vec4 u_Color;
uniform vec4 u_TextureSize; // xy = width/height, zw = 1/width, 1/height
void main()
{
    vec2 pointUV = (floor(v_texcoord0 * u_TextureSize.xy) + 0.5) * u_TextureSize.zw;
    vec4 texColor = texture2DLod(u_Texture, pointUV, 0.0);
    vec4 outColor = texColor * u_Color;
    gl_FragColor  = vec4(outColor.rgb * outColor.a, outColor.a);
}