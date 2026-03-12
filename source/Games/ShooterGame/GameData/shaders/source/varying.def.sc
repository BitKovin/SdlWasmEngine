vec2 v_texcoord0     : TEXCOORD0 = vec2(0.0, 0.0);
vec2 v_texcoord1     : TEXCOORD1 = vec2(0.0, 0.0);
vec3 v_normal        : TEXCOORD2 = vec3(0.0, 1.0, 0.0);
vec4 v_world         : TEXCOORD3 = vec4(0.0, 0.0, 0.0, 1.0);
vec4 v_color0        : COLOR0    = vec4(1.0, 1.0, 1.0, 1.0);
vec3 v_light         : TEXCOORD8 = vec3(0.0, 0.0, 0.0);

vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL0;
vec4 a_color0    : COLOR0;
vec2 a_texcoord0 : TEXCOORD0;
vec3 a_smoothNormal : TEXCOORD1;
vec2 a_texcoord2 : TEXCOORD2;
vec4 a_indices   : BLENDINDICES0;
vec4 a_weight    : BLENDWEIGHT0;
