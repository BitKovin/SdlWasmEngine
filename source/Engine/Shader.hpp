#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Logger.hpp"
#include "gl.h"
#include "glm.h"
#include "utility/hashed_string.hpp"

#include "Texture.hpp"
#include "FileSystem/FileSystem.h"
#include "malloc_override.h"
#include "Helpers/StringHelper.h"
using namespace std;

enum ShaderType
{
    VertexShader,
    PixelShader
};

// Forward declare so Shader can reference ShaderProgram and vice-versa
class ShaderProgram;

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
    std::string filePath = "";
    ShaderType shaderType = ShaderType::PixelShader;

    // List of programs that currently have this shader attached (registered via ShaderProgram::AttachShader).
    std::vector<ShaderProgram*> attachedPrograms;

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

    static Shader* FromFile(const char* filePath, ShaderType shaderType, bool autoCompile = true)
    {

        std::string pathStd = std::string(filePath);

		Logger::Log("Loading shader from file: " + pathStd);

        auto optimizedDir = pathStd;// StringHelper::Replace(pathStd, "GameData/shaders/", "GameData/shaders/.optimized/");

		Logger::Log("Loading optimized shader from file: " + std::string(optimizedDir));

        Shader* output = FromCode(FileSystemEngine::ReadFile(optimizedDir).c_str(), shaderType, autoCompile);
        output->filePath = filePath;

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
            Logger::Log(filePath);
            Logger::Log("Shader compilation failed:\n" + infoLog);
            Logger::Log(shaderCode);
        }
    }

    // Register/unregister programs that use this shader
    void RegisterProgram(ShaderProgram* prog)
    {
        if (std::find(attachedPrograms.begin(), attachedPrograms.end(), prog) == attachedPrograms.end())
            attachedPrograms.push_back(prog);
    }

    void UnregisterProgram(ShaderProgram* prog)
    {
        auto it = std::find(attachedPrograms.begin(), attachedPrograms.end(), prog);
        if (it != attachedPrograms.end())
            attachedPrograms.erase(it);
    }

    // Hot reloads shader source (from file if filePath is set, otherwise uses stored shaderCode).
    // Returns true if reload succeeded and programs were relinked, false if compilation or linking failed
    // (in case of failure the old shader stays attached).
    bool Reload();
};

class ShaderProgram
{

private:
    std::unordered_map<hashed_string, GLuint> m_textureUnits;
    GLuint m_currentUnit = 0;
    GLuint m_maxTextureUnits = 16; // Will be initialized from GL



public:
    GLuint program;
    std::vector<GLAttribute> attributes;  // Stores shader attributes.
    std::unordered_map<hashed_string, GLint> uniformLocations; // Cache for uniform locations.

    std::unordered_map<hashed_string, std::string> textureBindings;

    // Keep track of Shader* that are attached to this program (so we can unregister on destruction).
    std::vector<Shader*> attachedShaders;

    string name;

    bool AllowMissingUniforms = true; // keep your original semantics (you had 'true' earlier)

    ShaderProgram() {
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&m_maxTextureUnits);
        program = glCreateProgram();
    }

    ~ShaderProgram()
    {
        // unregister this program from attached shaders
        for (auto* s : attachedShaders)
        {
            if (s) s->UnregisterProgram(this);
        }

        if (program != 0)
            glDeleteProgram(program);
    }

    // Attaches a compiled shader to the program.
    ShaderProgram* AttachShader(Shader* shader)
    {
        if (!shader) return this;

        glAttachShader(program, shader->shaderPointer);

        // remember locally
        if (std::find(attachedShaders.begin(), attachedShaders.end(), shader) == attachedShaders.end())
            attachedShaders.push_back(shader);

        // register this program with the shader so Shader::Reload can find all programs using it
        shader->RegisterProgram(this);

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

        textureBindings = ParseAllTextureBindings();

        return this;
    }

    // Activates the program.
    void UseProgram();

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
    GLint GetUniformLocation(const hashed_string& name)
    {
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end())
            return it->second;

        GLint location = glGetUniformLocation(program, name.c_str());

        uniformLocations[name] = location;

        if (location >= 0)
            return location;



        if (AllowMissingUniforms == false)
            Logger::Log("Warning: Uniform \"" + name.str() + "\" not found in program " + std::to_string(program) + ".");

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

    void SetCubemapTexture(const std::string& name, GLuint texture) {
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

        // Bind cubemap texture and update uniform
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glUniform1i(location, unit);
    }

    void SetTexture(const hashed_string& name, Texture* texture) {
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
        glBindTexture(GL_TEXTURE_2D, texture == nullptr? 0 : texture->getID());
        glUniform1i(location, unit);
    }


    // === Uniform setting functions with cached locations ===
    
    // Set uniform integer
    void SetUniform(const std::string& name, int value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1) 
            Logger::Log("Using int uniform. They are not allowed during transition process");
		SetUniform(name, static_cast<float>(value));
    }

    void SetUniform(const std::string& name, bool value)
    {
		SetUniform(name, value ? 1.0f : 0.0f);
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

    // Set uniform array of floats
    void SetUniform(const std::string& name, const std::vector<float>& values)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1fv(location, static_cast<GLsizei>(values.size()), values.data());
    }

    // Set uniform array of vec2
    void SetUniform(const std::string& name, const std::vector<glm::vec2>& values)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2fv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(values[0]));
    }

    // Set uniform array of vec3
    void SetUniform(const std::string& name, const std::vector<glm::vec3>& values)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3fv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(values[0]));
    }

    // Set uniform array of vec4
    void SetUniform(const std::string& name, const std::vector<glm::vec4>& values)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4fv(location, static_cast<GLsizei>(values.size()), glm::value_ptr(values[0]));
    }

    // Set uniform array of mat2
    void SetUniform(const std::string& name, const std::vector<glm::mat2>& values)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix2fv(location,
                static_cast<GLsizei>(values.size()),
                GL_FALSE,
                glm::value_ptr(values[0]));
    }

    // Set uniform array of mat3
    void SetUniform(const std::string& name, const std::vector<glm::mat3>& values)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix3fv(location,
                static_cast<GLsizei>(values.size()),
                GL_FALSE,
                glm::value_ptr(values[0]));
    }

    // Set uniform array of mat4 
    void SetUniform(const std::string& name, const std::vector<glm::mat4>& values)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix4fv(location,
                static_cast<GLsizei>(values.size()),
                GL_FALSE,
                glm::value_ptr(values[0]));
    }

    

    static std::unordered_map<hashed_string, std::string> ParseTextureBindings(const std::string& shaderCode);

    std::unordered_map<hashed_string, std::string> ParseAllTextureBindings() const;

    void ParseShaders();

    void ApplyTextureBindings();

};
