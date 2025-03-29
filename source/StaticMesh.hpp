#pragma once

#include <vector>

#include "ShaderManager.h"

#include "IDrawMesh.h"

#include "VertexData.h"

#include "MathHelper.hpp"

//#include "MeshHelper.h"

using namespace std;

class StaticMesh : IDrawMesh
{
public:

	VertexArrayObject* vao = nullptr;

	StaticMesh()
	{

	}
	~StaticMesh()
	{

	}

	void DrawForward(mat4x4 view, mat4x4 projection, GLuint texture)
	{

		ShaderProgram* shader_program = ShaderManager::GetShaderProgram();

		shader_program->UseProgram();

		shader_program->SetTexture("u_texture", texture);

		vec3 pos = vec3(0, 0, 2);

		vec3 rot = vec3(0);

		mat4x4 world = scale(vec3(1)) * MathHelper::GetRotationMatrix(rot) * translate(pos);

		shader_program->SetUniform("view", view);
		shader_program->SetUniform("projection", projection);

		shader_program->SetUniform("world", world);


		vao->Bind();

		glDrawElements(GL_TRIANGLES, vao->IndexCount, GL_UNSIGNED_INT, 0);

	}

	void DrawDepth(mat4x4 viewProjection)
	{

	}

	void DrawShadow(mat4x4 viewProjection)
	{

	}

	


private:

};