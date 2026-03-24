$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(u_Texture, 0);

uniform vec4 u_Color;

void main()
{
    vec4 texColor = texture2D(u_Texture, v_texcoord0);
    vec4 outColor = texColor * u_Color;
    gl_FragColor  = vec4(outColor.rgb * outColor.a, outColor.a);
}
