#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utility/hashed_string.hpp"
#include "Texture.hpp"
#include "Logger.hpp"
#include "FileSystem/FileSystem.h"
#include "malloc_override.h"

// ---------------------------------------------------------------------------
// UniformMeta — per-uniform bookkeeping
// ---------------------------------------------------------------------------
struct UniformMeta
{
    enum class Kind { Vec4, Mat3, Mat4, Sampler };

    Kind              kind = Kind::Vec4;
    bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
    uint8_t           samplerSlot = 0;
    uint16_t          num = 1;   // array element count
};



class Shader
{
public:
    std::string name;
    bool        AllowMissingUniforms = true;

    // uniform name -> asset path, populated from @texture annotations in .sh sources
    std::unordered_map<hashed_string, std::string> textureBindings;

    // -----------------------------------------------------------------------
    // Factory
    // -----------------------------------------------------------------------

    // Create from compiled binary base-names (no extension, no platform path).
    // e.g. Shader::FromFiles("vs_mesh", "fs_mesh")
    // Compiled binaries are resolved from: GameData/shaders/compiled/<platform>/<renderer>/
    // Source files are resolved from:      GameData/shaders/source/
    static Shader* FromFiles(const char* vsName, const char* fsName);

    ~Shader();

    // -----------------------------------------------------------------------
    // Uniform setters — scalar / vector / matrix
    // -----------------------------------------------------------------------
    // bgfx maps everything float to Vec4 or Mat3/Mat4.
    // Smaller types (float, vec2, vec3, mat2) are padded to fit.

    void SetUniform(const std::string& uname, int   value);
    void SetUniform(const std::string& uname, bool  value);
    void SetUniform(const std::string& uname, float value);

    void SetUniform(const std::string& uname, const glm::vec2& value);
    void SetUniform(const std::string& uname, const glm::vec3& value);
    void SetUniform(const std::string& uname, const glm::vec4& value);

    // mat2 is packed into a single Vec4 (4 floats)
    void SetUniform(const std::string& uname, const glm::mat2& value);
    // mat3 uses bgfx Mat3 uniform type (3 Vec4 rows)
    void SetUniform(const std::string& uname, const glm::mat3& value);
    // mat4 uses bgfx Mat4 uniform type
    void SetUniform(const std::string& uname, const glm::mat4& value);

    // -----------------------------------------------------------------------
    // Uniform setters — arrays
    // -----------------------------------------------------------------------
    // Each float/vec2/vec3 element is padded into a Vec4 slot.
    // mat4 arrays map directly to Mat4 arrays.

    void SetUniform(const std::string& uname, const std::vector<float>& values);
    void SetUniform(const std::string& uname, const std::vector<glm::vec2>& values);
    void SetUniform(const std::string& uname, const std::vector<glm::vec3>& values);
    void SetUniform(const std::string& uname, const std::vector<glm::vec4>& values);
    void SetUniform(const std::string& uname, const std::vector<glm::mat2>& values);
    void SetUniform(const std::string& uname, const std::vector<glm::mat3>& values);
    void SetUniform(const std::string& uname, const std::vector<glm::mat4>& values);

    // -----------------------------------------------------------------------
    // Texture setters
    // -----------------------------------------------------------------------
    void SetTexture(const std::string& uname, bgfx::TextureHandle texture);
    void SetTexture(const hashed_string& uname, Texture* texture);
    void SetCubemapTexture(const std::string& uname, bgfx::TextureHandle texture);

    // -----------------------------------------------------------------------
    // Draw
    // -----------------------------------------------------------------------

    // Resolve @texture bindings, apply them, and prepare for submission.
    // Call this once per draw call before Submit().
    void UseProgram();

    // Submit draw call to bgfx view.
    void Submit(uint16_t viewId) const;

    // -----------------------------------------------------------------------
    // Hot reload
    // -----------------------------------------------------------------------

    // Destroys and recreates the program from disk.
    // All previously set uniforms remain valid because handles are
    // re-created with the same names during reflection.
    bool Reload();

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    bgfx::ProgramHandle GetProgram() const { return m_program; }
    bool IsValid()                   const { return bgfx::isValid(m_program); }

    // -----------------------------------------------------------------------
    // Texture-binding helpers (used by UseProgram / Reload)
    // -----------------------------------------------------------------------
    static std::unordered_map<hashed_string, std::string>
        ParseTextureBindings(const std::string& sourceCode);

    std::unordered_map<hashed_string, std::string>
        ParseAllTextureBindings() const;

    void ApplyTextureBindings();

private:
    Shader() = default;

    // -----------------------------------------------------------------------
    // Internal state
    // -----------------------------------------------------------------------
    bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;

    // name -> uniform metadata (handle, kind, sampler slot, array count)
    std::unordered_map<std::string, UniformMeta> m_uniforms;

    // stored for Reload()
    std::string m_vsName;
    std::string m_fsName;

    // stored for ParseAllTextureBindings()
    std::string m_vsSourcePath;
    std::string m_fsSourcePath;

    // -----------------------------------------------------------------------
    // Missing-texture fallback (shared across all Shader instances)
    // -----------------------------------------------------------------------
    static bgfx::TextureHandle s_missingTexture;
    static void                EnsureMissingTexture();

    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------
    static bool        IsBgfxBuiltin(const std::string& uname);
    static std::string ResolveCompiledPath(const std::string& shaderName);
    static std::string ResolveSourcePath(const std::string& shaderName);
    static bgfx::ShaderHandle LoadShaderBinary(const std::string& shaderName);

    // Reflect uniforms from both shader stages, populate m_uniforms.
    void ReflectUniforms(bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh);

    // Lookup helper — returns nullptr and optionally logs if not found.
    const UniformMeta* FindUniform(const std::string& uname) const;
};