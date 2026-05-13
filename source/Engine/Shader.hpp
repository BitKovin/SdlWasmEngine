#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <limits>

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

    Kind                kind        = Kind::Vec4;
    bgfx::UniformHandle handle      = BGFX_INVALID_HANDLE;
    uint8_t             samplerSlot = 0;
    uint16_t            num         = 1;   // array element count
    uint16_t            index       = 0;   // position in m_uniformList
};

// ---------------------------------------------------------------------------
// UniformEntry — buffered uniform value
//
// Hot-path optimisation: 16 floats of inline storage covers every non-array
// uniform type (vec4 = 4, mat3 = 12, mat4 = 16) without touching the heap.
// heapData is populated only when the total float count exceeds 16 (i.e. array
// uniforms with more than one mat4 element, or >4 vec4 elements, etc.).
//
// The `set` flag replaces the old pattern of calling unordered_map::clear()
// every frame: UseProgram() just loops through the vector and flips flags,
// which is a cache-friendly memset-like scan with no allocation/deallocation.
// ---------------------------------------------------------------------------
struct UniformEntry
{
    static constexpr int kInline = 16;

    float              inlineData[kInline] = {};
    std::vector<float> heapData;             // only for total floats > kInline
    uint16_t           num = 0;
    bool               set = false;

    float*       data()       { return heapData.empty() ? inlineData : heapData.data(); }
    const float* data() const { return heapData.empty() ? inlineData : heapData.data(); }

    // Write `floatCount` packed floats from `src`, routing to inline or heap.
    void assign(const float* src, size_t floatCount, uint16_t n)
    {
        if (floatCount <= static_cast<size_t>(kInline))
        {
            heapData.clear();
            std::memcpy(inlineData, src, floatCount * sizeof(float));
        }
        else
        {
            heapData.assign(src, src + floatCount);
        }
        num = n;
        set = true;
    }
};

struct DefaultUniformValue {
    std::vector<float> data;
};

// ---------------------------------------------------------------------------
// TextureEntry — buffered texture binding
// ---------------------------------------------------------------------------
struct TextureEntry
{
    uint8_t             slot          = 0;
    bgfx::UniformHandle samplerHandle = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle texture       = BGFX_INVALID_HANDLE;
    bool                set           = false;
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
    static Shader* FromFiles(const char* vsName, const char* fsName);

    ~Shader();

    // -----------------------------------------------------------------------
    // Uniform setters — scalar / vector / matrix
    // -----------------------------------------------------------------------
    void SetUniform(const std::string& uname, int   value);
    void SetUniform(const std::string& uname, bool  value);
    void SetUniform(const std::string& uname, float value);

    void SetUniform(const std::string& uname, const glm::vec2& value);
    void SetUniform(const std::string& uname, const glm::vec3& value);
    void SetUniform(const std::string& uname, const glm::vec4& value);

    void SetUniform(const std::string& uname, const glm::mat2& value);
    void SetUniform(const std::string& uname, const glm::mat3& value);
    void SetUniform(const std::string& uname, const glm::mat4& value);

    // -----------------------------------------------------------------------
    // Uniform setters — arrays
    // -----------------------------------------------------------------------
    void SetUniform(const std::string& uname, const std::vector<float>&      values);
    void SetUniform(const std::string& uname, const std::vector<glm::vec2>&  values);
    void SetUniform(const std::string& uname, const std::vector<glm::vec3>&  values);
    void SetUniform(const std::string& uname, const std::vector<glm::vec4>&  values);
    void SetUniform(const std::string& uname, const std::vector<glm::mat2>&  values);
    void SetUniform(const std::string& uname, const std::vector<glm::mat3>&  values);
    void SetUniform(const std::string& uname, const std::vector<glm::mat4>&  values);

    // -----------------------------------------------------------------------
    // Texture setters
    // -----------------------------------------------------------------------
    void SetTexture(const std::string&   uname, bgfx::TextureHandle texture);
    void SetTexture(const std::string&   uname, int texture);
    void SetTexture(const hashed_string& uname, Texture* texture);
    void SetCubemapTexture(const std::string& uname, bgfx::TextureHandle texture);

    // -----------------------------------------------------------------------
    // Draw
    // -----------------------------------------------------------------------
    void UseProgram();
    void Submit(uint16_t viewId) const;

    // -----------------------------------------------------------------------
    // Hot reload
    // -----------------------------------------------------------------------
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

    std::unordered_map<std::string, uint8_t> ParseSamplerSlots(const std::string& source);
    std::unordered_map<std::string, uint8_t> ParseAllSamplerSlots();

    void ParseDefaultUniforms(const std::string& source);

    std::unordered_map<hashed_string, std::string>
        ParseAllTextureBindings();

    void ApplyTextureBindings();

private:
    Shader() = default;

    static constexpr uint16_t kInvalidIndex = std::numeric_limits<uint16_t>::max();

    // -----------------------------------------------------------------------
    // Internal state
    // -----------------------------------------------------------------------
    bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;

    // name -> index into m_uniformList.
    // This is the only string-keyed map left; it is consulted once per
    // SetUniform / SetTexture call, not in the hot FlushBuffers path.
    std::unordered_map<std::string, uint16_t> m_uniformIndex;

    // Flat, index-stable list of all reflected uniforms.
    std::vector<UniformMeta> m_uniformList;

    // Indices (into m_uniformList) of Sampler-kind uniforms only.
    // Kept separate so FlushBuffers can iterate just the sampler subset.
    std::vector<uint16_t> m_samplerIndices;

    // stored for Reload()
    std::string m_vsName;
    std::string m_fsName;

    // stored for ParseAllTextureBindings()
    std::string m_vsSourcePath;
    std::string m_fsSourcePath;

    // -----------------------------------------------------------------------
    // Deferred uniform / texture buffers
    // Indexed 1:1 with m_uniformList — direct array access, no string lookup.
    // Marked mutable so Submit() const can flush them.
    //
    // UseProgram() resets the `set` flag rather than calling clear(), avoiding
    // all allocation/deallocation churn on every model boundary.
    // -----------------------------------------------------------------------
    mutable std::vector<UniformEntry> m_uniformBuffer;
    mutable std::vector<TextureEntry> m_textureBuffer;

    // -----------------------------------------------------------------------
    // Pre-resolved data — computed once at load / reload, reused every frame.
    // -----------------------------------------------------------------------

    // @texture binding pre-resolved to buffer indices so ApplyTextureBindings()
    // performs zero string operations per frame.
    struct ResolvedBinding
    {
        uint16_t    samplerIdx;  // index into m_uniformList / m_textureBuffer
        uint16_t    sizeIdx;     // index for <name>_size uniform; kInvalidIndex if absent
        std::string path;        // asset path passed to AssetRegistry
    };
    std::vector<ResolvedBinding> m_resolvedBindings;

    // Default uniform values pre-resolved to buffer indices + packed float data.
    // Defaults are always a single vec4 so four floats suffice.
    struct ResolvedDefault
    {
        uint16_t idx;
        float    data[4];
    };
    std::vector<ResolvedDefault> m_resolvedDefaults;

    // Raw default values (populated by ParseDefaultUniforms, consumed by BuildResolvedData).
    std::unordered_map<std::string, DefaultUniformValue> m_defaultUniforms;

    // Derives m_resolvedBindings and m_resolvedDefaults from textureBindings /
    // m_defaultUniforms after reflection and source parsing are complete.
    // Must be called after every ReflectUniforms + ParseAllTextureBindings pair.
    void BuildResolvedData();

    // Flush all buffered uniforms and textures to bgfx state (called by Submit).
    void FlushBuffers() const;

    // -----------------------------------------------------------------------
    // Shared texture fallbacks (all Shader instances)
    // -----------------------------------------------------------------------
    static bgfx::TextureHandle s_missingTexture; // magenta — set but invalid handle
    static bgfx::TextureHandle s_blackTexture;   // black   — sampler with no binding
    static void                EnsureMissingTexture();

    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------
    static bool        IsBgfxBuiltin(const std::string& uname);
    static std::string ResolveCompiledPath(const std::string& shaderName);
    static std::string ResolveSourcePath(const std::string& shaderName);
    static bgfx::ShaderHandle LoadShaderBinary(const std::string& shaderName);

    // Reflect uniforms from both shader stages; populates m_uniformIndex,
    // m_uniformList, m_samplerIndices, m_uniformBuffer, m_textureBuffer.
    void ReflectUniforms(bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh);

    // Returns the index into m_uniformList, or kInvalidIndex if not found.
    // Logs a warning/error when AllowMissingUniforms is false or the handle is bad.
    uint16_t FindUniformIndex(const std::string& uname) const;
};
