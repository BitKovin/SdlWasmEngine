#version 300 es

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoordinate;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;
	
uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
	
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
	
out vec2 v_texcoord;
	
mat4 GetBoneTransforms()
{

	mat4 identity = mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);


	float sum = weights.x + weights.y + weights.z + weights.w;

	if (sum < 0.05f)
		return identity;

	mat4 mbones =
		finalBonesMatrices[boneIds.x] * weights.x / sum +
		finalBonesMatrices[boneIds.y] * weights.y / sum +
		finalBonesMatrices[boneIds.z] * weights.z / sum +
		finalBonesMatrices[boneIds.w] * weights.w / sum;

	return mbones;
}

void main()
{

    mat4 boneTrans = GetBoneTransforms();

    mat4 vertWorldTrans = world * boneTrans;

    gl_Position = projection * view * vertWorldTrans * vec4(Position, 1.0);
    v_texcoord = TextureCoordinate;
}