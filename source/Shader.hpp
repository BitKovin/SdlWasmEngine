#pragma once

#include <string>

#include "gl.h"

using namespace std;

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	string shaderCode = "";

	static ShaderProgram FromCode()
	{
	}

private:

	GLuint shaderPointer = 0;

};

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::~ShaderProgram()
{
}