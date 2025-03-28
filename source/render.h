#pragma once

#include <string>
#include <vector>
#include <utility>
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
    Shader() {}

    GLuint shaderPointer = 0;
    std::string shaderCode = "";

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
};

class ShaderProgram
{
public:
    GLuint program;
    std::vector<std::pair<std::string, GLint>> attributes;
    std::vector<std::pair<std::string, GLint>> uniforms;

    ShaderProgram()
    {
        program = glCreateProgram();
    }

    // Attaches a compiled shader to the program.
    ShaderProgram* AttachShader(Shader shader)
    {
        glAttachShader(program, shader.shaderPointer);
        return this;
    }

    // Links the program.
    ShaderProgram* LinkProgram()
    {
        glLinkProgram(program);
        return this;
    }

    // Activates the program.
    void UseProgram()
    {
        glUseProgram(program);
    }

    // Fills the attributes vector by querying the linked program.
    void FillAttributes()
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
            attributes.push_back({ string(name), location });
        }
    }

    // Fills the uniforms vector by querying the linked program.
    void FillUniforms()
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
            uniforms.push_back({ string(name), location });
        }
    }

    // === Uniform setting functions ===

    // Sets a single integer uniform.
    void SetUniform1i(const std::string& name, int value)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniform1i(location, value);
    }

    // Sets a single float uniform.
    void SetUniform1f(const std::string& name, float value)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniform1f(location, value);
    }

    // Sets a 2-component float uniform (vec2).
    void SetUniform2f(const std::string& name, float v0, float v1)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniform2f(location, v0, v1);
    }

    // Sets a 3-component float uniform (vec3).
    void SetUniform3f(const std::string& name, float v0, float v1, float v2)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniform3f(location, v0, v1, v2);
    }

    // Sets a 4-component float uniform (vec4).
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniform4f(location, v0, v1, v2, v3);
    }

    // Sets a 2-component integer uniform (ivec2).
    void SetUniform2i(const std::string& name, int v0, int v1)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniform2i(location, v0, v1);
    }

    // Sets a 3-component integer uniform (ivec3).
    void SetUniform3i(const std::string& name, int v0, int v1, int v2)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniform3i(location, v0, v1, v2);
    }

    // Sets a 4-component integer uniform (ivec4).
    void SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniform4i(location, v0, v1, v2, v3);
    }

    // Sets a 2x2 float matrix uniform (mat2).
    void SetUniformMatrix2fv(const std::string& name, const float* mat, GLsizei count = 1, GLboolean transpose = GL_FALSE)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform matrix \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniformMatrix2fv(location, count, transpose, mat);
    }

    // Sets a 3x3 float matrix uniform (mat3).
    void SetUniformMatrix3fv(const std::string& name, const float* mat, GLsizei count = 1, GLboolean transpose = GL_FALSE)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform matrix \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniformMatrix3fv(location, count, transpose, mat);
    }

    // Sets a 4x4 float matrix uniform (mat4).
    void SetUniformMatrix4fv(const std::string& name, const float* mat, GLsizei count = 1, GLboolean transpose = GL_FALSE)
    {
        GLint location = glGetUniformLocation(program, name.c_str());
        if (location == -1)
        {
            Logger::Log("Warning: Uniform matrix \"" + name + "\" not found in program " + std::to_string(program) + ".");
            return;
        }
        glUniformMatrix4fv(location, count, transpose, mat);
    }
};
