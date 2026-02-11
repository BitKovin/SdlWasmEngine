#version 300 es

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoordinate;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;
layout(location = 7) in vec3 smoothNormals;
layout(location = 8) in vec4 color;
	
uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;
	
uniform mat4 lightMatrix1;
uniform mat4 lightMatrix2;
uniform mat4 lightMatrix3;
uniform mat4 lightMatrix4;

uniform float brightness;

uniform float viewmodelScaleFactor;

const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
	
uniform bool isViewmodel;

out vec2 v_texcoord;
out vec4 v_color;
out vec3 v_normal;
out vec3 v_worldPosition;
out vec4 v_clipPosition;

out vec4 v_shadowCoords1;
out vec4 v_shadowCoords2;
out vec4 v_shadowCoords3;
out vec4 v_shadowCoords4;

out vec3 v_light;
	




mat4 GetBoneTransforms()
{

	mat4 identity = mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0);


	float sum = weights.x + weights.y + weights.z + weights.w;

	if (sum < 0.01f)
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

    v_clipPosition = projection * view * vertWorldTrans * vec4(Position, 1.0);

	if(isViewmodel)
		v_clipPosition.z*=0.02 * viewmodelScaleFactor;

	gl_Position = v_clipPosition;
	v_worldPosition = (vertWorldTrans * vec4(Position, 1.0)).xyz;

    mat3 normalMatrix = transpose(inverse(mat3(vertWorldTrans)));

    // Flip normal if scaling flips handedness
    float det = determinant(mat3(vertWorldTrans));
    if(det < 0.0)
        normalMatrix = -normalMatrix;

    v_normal = normalize(normalMatrix * Normal);



	v_color = vec4(brightness,brightness,brightness, 1) * color;

    v_texcoord = TextureCoordinate;

	v_shadowCoords1 = lightMatrix1 * vertWorldTrans * vec4(Position, 1.0);
	v_shadowCoords2 = lightMatrix2 * vertWorldTrans * vec4(Position, 1.0);
	v_shadowCoords3 = lightMatrix3 * vertWorldTrans * vec4(Position, 1.0);
	v_shadowCoords4 = lightMatrix4 * vertWorldTrans * vec4(Position, 1.0);

	v_light = vec3(0);



}