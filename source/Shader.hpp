#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Logger.hpp"
#include "gl.h"

using namespace std;

enum ShaderType
{
    VertexShader,
    PixelShader
};

// Struct for storing OpenGL attribute information.
struct GLAttribute
{
    std::string name;  // Attribute name.
    GLenum type = -1;       // Data type (e.g., GL_FLOAT, GL_INT).
    GLint size = -1;        // Size (number of components, or array size).
    GLint location = -1;    // Location within the shader program.
};

class Shader
{
public:
    Shader() {}

    GLuint shaderPointer = 0;
    std::string shaderCode = "";

    ShaderType shaderType = ShaderType::PixelShader;

    // Creates a Shader object from source code.
    static Shader FromCode(const char* code, ShaderType shaderType, bool autoCompile = true)
    {
        Shader output;
        GLuint glShaderType = (shaderType == VertexShader) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;

        output.shaderCode = code;
        const char* cCode = code;

        output.shaderPointer = glCreateShader(glShaderType);

        output.shaderType = shaderType;

        glShaderSource(output.shaderPointer, 1, &cCode, NULL);

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
    std::vector<GLAttribute> attributes;  // Stores shader attributes.
    std::unordered_map<std::string, GLint> uniformLocations; // Cache for uniform locations.

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

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success == GL_FALSE)
        {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::string infoLog(logLength, ' ');
            glGetProgramInfoLog(program, logLength, &logLength, &infoLog[0]);
            Logger::Log("Shader program linking failed:\n" + infoLog);
        }

        FillAttributes();
        CacheUniformLocations();
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

            GLAttribute atribute;

            atribute.name = name;
            atribute.type = type;
            atribute.size = size;
            atribute.location = location;

            attributes.push_back(atribute);
        }
    }

    // Caches uniform locations to avoid redundant glGetUniformLocation calls.
    void CacheUniformLocations()
    {
        uniformLocations.clear();
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
            uniformLocations[name] = location;
        }
    }

    // Retrieves a cached uniform location.
    GLint GetUniformLocation(const std::string& name)
    {
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
            return it->second;

        Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");
        return -1;
    }

    // === Uniform setting functions with cached locations ===

    void SetUniform1i(const std::string& name, int value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform1i(location, value);
    }

    void SetUniform1f(const std::string& name, float value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform1f(location, value);
    }

    void SetUniform2f(const std::string& name, float v0, float v1)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform2f(location, v0, v1);
    }

    void SetUniform3f(const std::string& name, float v0, float v1, float v2)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform3f(location, v0, v1, v2);
    }

    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform4f(location, v0, v1, v2, v3);
    }

    void SetUniform2i(const std::string& name, int v0, int v1)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform2i(location, v0, v1);
    }

    void SetUniform3i(const std::string& name, int v0, int v1, int v2)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform3i(location, v0, v1, v2);
    }

    void SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform4i(location, v0, v1, v2, v3);
    }

    void SetUniformMatrix2fv(const std::string& name, const float* mat, GLsizei count = 1, GLboolean transpose = GL_FALSE)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniformMatrix2fv(location, count, transpose, mat);
    }

    void SetUniformMatrix3fv(const std::string& name, const float* mat, GLsizei count = 1, GLboolean transpose = GL_FALSE)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniformMatrix3fv(location, count, transpose, mat);
    }

    void SetUniformMatrix4fv(const std::string& name, const float* mat, GLsizei count = 1, GLboolean transpose = GL_FALSE)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniformMatrix4fv(location, count, transpose, mat);
    }
};
