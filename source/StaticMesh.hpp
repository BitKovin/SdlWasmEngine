#pragma once

#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"
#include "Camera.h"

#include "MeshHelper.h"

using namespace std;

class StaticMesh : IDrawMesh
{
public:


	StaticMesh()
	{
		//buffer = GenerateCubeBuffer();
	}
	~StaticMesh()
	{

	}

	void DrawForward(mat4x4 viewProjection)
	{

		ShaderProgram* shader = nullptr;// ShaderManager::GetShaderProgram();

		shader->UseProgram();


		vec3 pos = vec3(0, 0, 2);

		vec3 rot = vec3();

		mat4x4 world = scale(vec3(1)) * MathHelper::GetRotationMatrix(rot) * translate(pos);

		shader->SetUniform("view", Camera::finalizedView);
		shader->SetUniform("projection", Camera::finalizedProjection);

		shader->SetUniform("world", world);

		//SetupVertexAttributes();

	}

	void DrawDepth(mat4x4 viewProjection)
	{

	}

	void DrawShadow(mat4x4 viewProjection)
	{

	}

	


private:

};