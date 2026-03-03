#include "Shader.hpp"

#include "AssetRegistry.h"

#include <regex>

bool Shader::Reload()
{
    // 1) obtain new source (file if available, otherwise stored source)
    std::string newSource;
    if (!filePath.empty())
        newSource = FileSystemEngine::ReadFile(filePath);
    else
        newSource = shaderCode;

    if (newSource.empty())
    {
        Logger::Log("Shader::Reload: source is empty for " + filePath);
        return false;
    }

    // 2) compile new shader object
    GLuint glShaderType = (shaderType == VertexShader) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    GLuint newShader = glCreateShader(glShaderType);
    const char* cSource = newSource.c_str();
    glShaderSource(newShader, 1, &cSource, nullptr);
    glCompileShader(newShader);

    GLint compileStatus = GL_FALSE;
    glGetShaderiv(newShader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        GLint logLength = 0;
        glGetShaderiv(newShader, GL_INFO_LOG_LENGTH, &logLength);
        std::string infoLog((logLength > 0) ? logLength : 1, ' ');
        glGetShaderInfoLog(newShader, logLength, &logLength, &infoLog[0]);
        Logger::Log(std::string("Shader compilation failed during reload:\n") + infoLog);
        Logger::Log(newSource);
        glDeleteShader(newShader);
        return false;
    }

    // 3) swap attempt across all programs that reference this shader
    GLuint oldShader = shaderPointer; // may be 0 if none previously
    std::vector<ShaderProgram*> modifiedPrograms; // programs we've successfully switched to newShader

    for (ShaderProgram* prog : attachedPrograms)
    {
        // detach old shader first (prevents two definitions like duplicate 'main')
        if (oldShader != 0)
            glDetachShader(prog->program, oldShader);

        // attach new shader and link
        glAttachShader(prog->program, newShader);
        glLinkProgram(prog->program);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(prog->program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE)
        {
            // gather and log the program info log for debugging
            GLint logLength = 0;
            glGetProgramiv(prog->program, GL_INFO_LOG_LENGTH, &logLength);
            std::string infoLog((logLength > 0) ? logLength : 1, ' ');
            glGetProgramInfoLog(prog->program, logLength, &logLength, &infoLog[0]);

            std::string shaderKind = (shaderType == VertexShader) ? "Vertex" : "Fragment";
            Logger::Log("Shader program linking failed while reloading shader:\n" + shaderKind + " info\n-------------\n" + infoLog);

            // ROLLBACK: restore previously modified programs to old shader
            for (ShaderProgram* mprog : modifiedPrograms)
            {
                // detach new, reattach old and relink
                glDetachShader(mprog->program, newShader);
                if (oldShader != 0)
                    glAttachShader(mprog->program, oldShader);

                glLinkProgram(mprog->program);

                GLint rbLink = GL_FALSE;
                glGetProgramiv(mprog->program, GL_LINK_STATUS, &rbLink);
                if (rbLink == GL_FALSE)
                {
                    GLint rbLogLen = 0;
                    glGetProgramiv(mprog->program, GL_INFO_LOG_LENGTH, &rbLogLen);
                    std::string rbLog((rbLogLen > 0) ? rbLogLen : 1, ' ');
                    glGetProgramInfoLog(mprog->program, rbLogLen, &rbLogLen, &rbLog[0]);
                    Logger::Log("Rollback relink failed for program " + std::to_string(mprog->program) + ":\n" + rbLog);
                }

                mprog->FillAttributes();
                mprog->CacheUniformLocations();
            }

            // Also rollback the current program (we detached old & attached new earlier)
            glDetachShader(prog->program, newShader);
            if (oldShader != 0)
                glAttachShader(prog->program, oldShader);

            glLinkProgram(prog->program);
            GLint rbLinkCur = GL_FALSE;
            glGetProgramiv(prog->program, GL_LINK_STATUS, &rbLinkCur);
            if (rbLinkCur == GL_FALSE)
            {
                GLint rbLogLen = 0;
                glGetProgramiv(prog->program, GL_INFO_LOG_LENGTH, &rbLogLen);
                std::string rbLog((rbLogLen > 0) ? rbLogLen : 1, ' ');
                glGetProgramInfoLog(prog->program, rbLogLen, &rbLogLen, &rbLog[0]);
                Logger::Log("Rollback relink failed for program " + std::to_string(prog->program) + ":\n" + rbLog);
            }

            prog->FillAttributes();
            prog->CacheUniformLocations();

            // delete the new shader object and abort
            glDeleteShader(newShader);
            Logger::Log("Shader::Reload aborted — link error(s). Keeping old shader.");
            return false;
        }

        // success for this program -> update caches & record it
        prog->FillAttributes();
        prog->CacheUniformLocations();
        prog->textureBindings = prog->ParseAllTextureBindings();
        modifiedPrograms.push_back(prog);
    }

    // 4) All programs linked successfully with newShader — commit the change
    shaderPointer = newShader;
    shaderCode = newSource;

    // delete old shader if present
    if (oldShader != 0)
        glDeleteShader(oldShader);

    Logger::Log("Shader::Reload succeeded for " + filePath);
    return true;
}

static GLuint gMissingTexture = 0;

static void CreateMissingTexture()
{
    glGenTextures(1, &gMissingTexture);
    glBindTexture(GL_TEXTURE_2D, gMissingTexture);

    uint32_t pink[4] = {
        0, 0,
        0, 0
    };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        2, 2, 0,
        GL_RGBA, GL_UNSIGNED_BYTE,
        pink);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void ShaderProgram::UseProgram()
{

    if (gMissingTexture == 0)
        CreateMissingTexture();

    // Unbind previously bound textures
    for (const auto& pair : m_textureUnits)
    {
        GLuint unit = pair.second;

        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, gMissingTexture);         // Unbind 2D texture
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);   // Unbind cube map
    }

    // Activate this shader program
    glUseProgram(program);

    ApplyTextureBindings();

}

std::unordered_map<hashed_string, std::string> ShaderProgram::ParseTextureBindings(const std::string& shaderCode)
{
    std::unordered_map<hashed_string, std::string> result;

    // Regex: match `uniform sampler2D <name>; // @texture <path>`
    std::regex re(R"(uniform\s+sampler2D\s+(\w+)\s*;.*@texture\s+([^\s]+))");

    std::smatch match;
    std::string::const_iterator searchStart(shaderCode.cbegin());

    while (std::regex_search(searchStart, shaderCode.cend(), match, re))
    {
        std::string uniformName = match[1].str();
        std::string texturePath = match[2].str();
        result[uniformName] = texturePath;

        searchStart = match.suffix().first;
    }

    return result;
}

std::unordered_map<hashed_string, std::string> ShaderProgram::ParseAllTextureBindings() const
{
    std::unordered_map<hashed_string, std::string> result;

    // Regex matches: uniform sampler2D myTex; // @texture path/to/file.png
    std::regex re(R"(uniform\s+(?:sampler2D|samplerCube)\s+(\w+)\s*;.*@texture\s+([^\s]+))");

    for (const Shader* s : attachedShaders)
    {
        if (!s) continue;

        const std::string& code = s->shaderCode;

        std::smatch match;
        std::string::const_iterator searchStart(code.cbegin());

        while (std::regex_search(searchStart, code.cend(), match, re))
        {
            std::string uniformName = match[1].str();
            std::string texturePath = match[2].str();

            // overwrite if duplicate uniform name across shaders
            result[uniformName] = texturePath;

            searchStart = match.suffix().first;
        }
    }

    return result;
}

void ShaderProgram::ParseShaders()
{

    // For each attached shader, parse bindings
    for (Shader* s : attachedShaders)
    {
        auto parsed = ParseTextureBindings(s->shaderCode);
        for (auto& kv : parsed)
        {
            textureBindings[kv.first] = kv.second;
        }
    }

}

void ShaderProgram::ApplyTextureBindings()
{

    for (const auto& pair : textureBindings)
    {
        const hashed_string& uniformName = pair.first;
        const std::string& path = pair.second;

        Texture* tex = AssetRegistry::GetTextureFromFile(path); 
        if (tex)
            SetTexture(uniformName, tex);
    }

}
