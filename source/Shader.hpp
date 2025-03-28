#pragma once

#include <string>
#include <vector>

#include "Logger.hpp"
#include "gl.h"

using namespace std;


enum ShaderType
{
    VertexShader,
    PixelShader
};

class Shader
{
public:

    Shader()
    {

    }

    GLuint shaderPointer = 0;

    // Shader source code.
    std::string shaderCode = "";

    // Vectors to hold attribute and uniform information (name and location).
    std::vector<std::pair<std::string, GLint>> attributes;
    std::vector<std::pair<std::string, GLint>> uniforms;

    // Creates a Shader object from source code.
    static Shader FromCode(const char* code, ShaderType shaderType, bool autoCompile = true)
    {
        Shader output;
        GLuint glShaderType = 0;

        switch (shaderType)
        {
        case VertexShader:
            glShaderType = GL_VERTEX_SHADER;
            break;
        case PixelShader:
            glShaderType = GL_FRAGMENT_SHADER;
            break;
        default:
            break;
        }

        output.shaderCode = code;
        const char* cCode = code;

        GLuint shader = glCreateShader(glShaderType);
        glShaderSource(shader, 1, &cCode, NULL);
        output.shaderPointer = shader;

        if (autoCompile)
            output.CompileShader();

        return output;
    }

    // Compiles the shader and logs compile errors if any.
    void CompileShader()
    {
        glCompileShader(shaderPointer);

        GLint success = 0;
        glGetShaderiv(shaderPointer, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            GLint logLength = 0;
            glGetShaderiv(shaderPointer, GL_INFO_LOG_LENGTH, &logLength);
            std::string infoLog(logLength, ' ');
            glGetShaderInfoLog(shaderPointer, logLength, &logLength, &infoLog[0]);
            Logger::Log("Shader compilation failed:\n" + infoLog);
        }
    }

    // Fills the attributes vector using a linked program.
    // It queries the active attributes of the provided program.
    void FillAttributes(GLuint program)
    {
        attributes.clear();
        GLint attributeCount = 0;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributeCount);

        char name[256];
        for (int i = 0; i < attributeCount; i++)
        {
            GLsizei length = 0;
            GLint size = 0;
            GLenum type = 0;
            glGetActiveAttrib(program, i, sizeof(name), &length, &size, &type, name);
            GLint location = glGetAttribLocation(program, name);
            attributes.push_back({ std::string(name), location });
        }
    }

    // Fills the uniforms vector using a linked program.
    // It queries the active uniforms of the provided program.
    void FillUniforms(GLuint program)
    {
        uniforms.clear();
        GLint uniformCount = 0;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

        char name[256];
        for (int i = 0; i < uniformCount; i++)
        {
            GLsizei length = 0;
            GLint size = 0;
            GLenum type = 0;
            glGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);
            GLint location = glGetUniformLocation(program, name);
            uniforms.push_back({ std::string(name), location });
        }
    }

private:

};

class ShaderProgram
{

private:

    


public:
   
    GLuint program;

    ShaderProgram()
    {
        program = glCreateProgram();
    }

    ShaderProgram* AttachShader(Shader shader)
    {
        glAttachShader(program, shader.shaderPointer);

        return this;
    }

    ShaderProgram* LinkProgram()
    {
        glLinkProgram(program);

        return this;
    }

    void UseProgram()
    {
        glUseProgram(program);
    }

};