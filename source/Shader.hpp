#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Logger.hpp"
#include "gl.h"
#include "glm.h"

#include "Texture.hpp"

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
    static Shader* FromCode(const char* code, ShaderType shaderType, bool autoCompile = true)
    {
        Shader* output = new Shader();
        GLuint glShaderType = (shaderType == VertexShader) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;

        output->shaderCode = code;
        const char* cCode = code;

        output->shaderPointer = glCreateShader(glShaderType);

        output->shaderType = shaderType;

        glShaderSource(output->shaderPointer, 1, &cCode, NULL);

        if (autoCompile)
            output->CompileShader();

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

private:
    std::unordered_map<std::string, GLuint> m_textureUnits;
    GLuint m_currentUnit = 0;
    GLuint m_maxTextureUnits = 16; // Will be initialized from GL


public:
    GLuint program;
    std::vector<GLAttribute> attributes;  // Stores shader attributes.
    std::unordered_map<std::string, GLint> uniformLocations; // Cache for uniform locations.

    bool AllowMissingUniforms = true;

    ShaderProgram() {
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&m_maxTextureUnits);
        program = glCreateProgram();
    }

    // Attaches a compiled shader to the program.
    ShaderProgram* AttachShader(Shader* shader)
    {
        glAttachShader(program, shader->shaderPointer);
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

        GLint location = glGetUniformLocation(program, name.c_str());

        uniformLocations[name] = location;

        if(location>0)
            return location;



        if(AllowMissingUniforms == false)
            Logger::Log("Warning: Uniform \"" + name + "\" not found in program " + std::to_string(program) + ".");

        return -1;
    }

    void SetTexture(const std::string& name, GLuint texture) {
        GLint location = GetUniformLocation(name);
        if (location == -1) return;

        // Find or assign texture unit
        auto it = m_textureUnits.find(name);
        if (it == m_textureUnits.end()) {
            if (m_currentUnit >= m_maxTextureUnits) {
                Logger::Log("Texture unit overflow! Maximum: " +
                    std::to_string(m_maxTextureUnits));
                return;
            }
            m_textureUnits[name] = m_currentUnit++;
        }

        GLuint unit = m_textureUnits[name];

        // Bind texture and update uniform
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(location, unit);
    }

    void SetTexture(const std::string& name, Texture* texture) {
        GLint location = GetUniformLocation(name);
        if (location == -1) return;

        // Find or assign texture unit
        auto it = m_textureUnits.find(name);
        if (it == m_textureUnits.end()) {
            if (m_currentUnit >= m_maxTextureUnits) {
                Logger::Log("Texture unit overflow! Maximum: " +
                    std::to_string(m_maxTextureUnits));
                return;
            }
            m_textureUnits[name] = m_currentUnit++;
        }

        GLuint unit = m_textureUnits[name];

        // Bind texture and update uniform
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture->getID());
        glUniform1i(location, unit);
    }

    // === Uniform setting functions with cached locations ===
    
        // Set uniform integer
    void SetUniform(const std::string& name, int value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform1i(location, value);
    }

    // Set uniform float
    void SetUniform(const std::string& name, float value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform1f(location, value);
    }

    // Set uniform vec2
    void SetUniform(const std::string& name, const glm::vec2& value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform2f(location, value.x, value.y);
    }

    // Set uniform vec3
    void SetUniform(const std::string& name, const glm::vec3& value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform3f(location, value.x, value.y, value.z);
    }

    // Set uniform vec4
    void SetUniform(const std::string& name, const glm::vec4& value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    // Set uniform mat2
    void SetUniform(const std::string& name, const glm::mat2& value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    // Set uniform mat3
    void SetUniform(const std::string& name, const glm::mat3& value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    // Set uniform mat4
    void SetUniform(const std::string& name, const glm::mat4& value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

};
