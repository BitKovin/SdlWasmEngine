#include "Shader.hpp"

#include "AssetRegistry.h"

#include <regex>
#include <cstring>
#include <unordered_set>

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
bgfx::TextureHandle Shader::s_missingTexture = BGFX_INVALID_HANDLE;

// ---------------------------------------------------------------------------
// Path resolution helpers
// ---------------------------------------------------------------------------

// Returns the platform + renderer subfolder for compiled shader binaries.
// e.g. "windows/dx11/"
static std::string GetPlatformRendererFolder()
{
    std::string platform;
#if defined(_WIN32)
    platform = "windows/";
#elif defined(__APPLE__)
    platform = "osx/";
#elif defined(__EMSCRIPTEN__)
    platform = "web/";
#elif defined(__ANDROID__)
    platform = "android/";
#elif defined(__linux__)
    platform = "linux/";
#else
    platform = "unknown/";
#endif

    std::string renderer;
    switch (bgfx::getRendererType())
    {
    case bgfx::RendererType::Direct3D11: renderer = "dx11/";   break;
    case bgfx::RendererType::Direct3D12: renderer = "dx12/";   break;
    case bgfx::RendererType::OpenGL:     renderer = "gl/";     break;
    case bgfx::RendererType::OpenGLES:   renderer = "gles/";   break;
    case bgfx::RendererType::Vulkan:     renderer = "spirv/";  break;
    case bgfx::RendererType::Metal:      renderer = "metal/";  break;
    case bgfx::RendererType::WebGPU:     renderer = "wgpu/";   break;
    default:                             renderer = "unknown/"; break;
    }

    return platform + renderer;
}

// Compiled binary: GameData/shaders/compiled/<platform>/<renderer>/<name>.bin
std::string Shader::ResolveCompiledPath(const std::string& shaderName)
{
    return "GameData/shaders/compiled/" + GetPlatformRendererFolder() + shaderName + ".bin";
}

// Source .sh file: GameData/shaders/source/<name>.sh
std::string Shader::ResolveSourcePath(const std::string& shaderName)
{
    return "GameData/shaders/source/" + shaderName + ".sh";
}

// Load a compiled binary and create a bgfx ShaderHandle.
bgfx::ShaderHandle Shader::LoadShaderBinary(const std::string& shaderName)
{
    const std::string path = ResolveCompiledPath(shaderName);
    Logger::Log("Loading shader binary: " + path);

    std::vector<uint8_t> data = FileSystemEngine::ReadFileBinary(path);
    if (data.empty())
    {
        Logger::Log("Shader::LoadShaderBinary — file empty or not found: " + path);
        return BGFX_INVALID_HANDLE;
    }

    // bgfx requires a null-terminated block
    const bgfx::Memory* mem = bgfx::alloc(uint32_t(data.size() + 1));
    memcpy(mem->data, data.data(), data.size());
    mem->data[data.size()] = '\0';

    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, path.c_str());
    return handle;
}

// ---------------------------------------------------------------------------
// Built-in uniform filter
// ---------------------------------------------------------------------------
bool Shader::IsBgfxBuiltin(const std::string& uname)
{
    static const std::unordered_set<std::string> builtins =
    {
        "u_view", "u_invView",
        "u_proj", "u_invProj",
        "u_viewProj", "u_invViewProj",
        "u_modelView", "u_invModelView",
        "u_modelViewProj",
        "u_viewRect", "u_viewTexel",
        "u_alphaRef4", "u_model"
    };
    return builtins.count(uname) != 0;
}

// ---------------------------------------------------------------------------
// Uniform reflection
// ---------------------------------------------------------------------------
void Shader::ReflectUniforms(bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh)
{
    m_uniforms.clear();

    uint8_t samplerSlot = 0;

    // Reflect both stages; merge results (fragment shader owns samplers).
    bgfx::ShaderHandle stages[2] = { vsh, fsh };

    for (bgfx::ShaderHandle stage : stages)
    {
        if (!bgfx::isValid(stage)) continue;

        uint16_t count = bgfx::getShaderUniforms(stage, nullptr, 0);
        if (count == 0) continue;

        std::vector<bgfx::UniformHandle> handles(count);
        bgfx::getShaderUniforms(stage, handles.data(), count);

        for (uint16_t i = 0; i < count; ++i)
        {
            bgfx::UniformInfo info;
            bgfx::getUniformInfo(handles[i], info);

            const std::string uname(info.name);

            if (IsBgfxBuiltin(uname))         continue;
            if (m_uniforms.count(uname))       continue; // already reflected

            UniformMeta u;
            // Re-create the uniform so bgfx knows the correct array size (num)
            u.handle = bgfx::createUniform(info.name, info.type, info.num);
            u.num = info.num;

            switch (info.type)
            {
            case bgfx::UniformType::Sampler:
                u.kind = UniformMeta::Kind::Sampler;
                u.samplerSlot = samplerSlot++;
                break;
            case bgfx::UniformType::Vec4:
                u.kind = UniformMeta::Kind::Vec4;
                break;
            case bgfx::UniformType::Mat3:
                u.kind = UniformMeta::Kind::Mat3;
                break;
            case bgfx::UniformType::Mat4:
                u.kind = UniformMeta::Kind::Mat4;
                break;
            default:
                // Unknown type — destroy handle and skip
                bgfx::destroy(u.handle);
                continue;
            }

            m_uniforms.emplace(uname, u);
        }
    }
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------
Shader* Shader::FromFiles(const char* vsName, const char* fsName)
{
    bgfx::ShaderHandle vsh = LoadShaderBinary(vsName);
    bgfx::ShaderHandle fsh = LoadShaderBinary(fsName);

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
    {
        Logger::Log(std::string("Shader::FromFiles — failed to load: ") + vsName + " / " + fsName);
        if (bgfx::isValid(vsh)) bgfx::destroy(vsh);
        if (bgfx::isValid(fsh)) bgfx::destroy(fsh);
        return nullptr;
    }

    Shader* s = new Shader();
    s->m_vsName = vsName;
    s->m_fsName = fsName;

    // Source paths for @texture annotation parsing
    s->m_vsSourcePath = ResolveSourcePath(vsName);
    s->m_fsSourcePath = ResolveSourcePath(fsName);

    // createProgram(vsh, fsh, destroyShaders=true)
    // bgfx takes ownership of shader handles when destroyShaders is true.
    // We must reflect BEFORE handing them to createProgram.
    s->ReflectUniforms(vsh, fsh);

    s->m_program = bgfx::createProgram(vsh, fsh, /*destroyShaders=*/true);

    if (!bgfx::isValid(s->m_program))
    {
        Logger::Log(std::string("Shader::FromFiles — bgfx::createProgram failed for: ") + vsName + " / " + fsName);
        delete s;
        return nullptr;
    }

    // Parse @texture annotations from source files
    s->textureBindings = s->ParseAllTextureBindings();

    return s;
}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
Shader::~Shader()
{
    // Destroy all created uniform handles
    for (auto& [n, u] : m_uniforms)
    {
        if (bgfx::isValid(u.handle))
            bgfx::destroy(u.handle);
    }

    if (bgfx::isValid(m_program))
        bgfx::destroy(m_program);
}

// ---------------------------------------------------------------------------
// Hot reload
// ---------------------------------------------------------------------------
bool Shader::Reload()
{
    bgfx::ShaderHandle vsh = LoadShaderBinary(m_vsName);
    bgfx::ShaderHandle fsh = LoadShaderBinary(m_fsName);

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
    {
        Logger::Log("Shader::Reload — binary load failed for: " + m_vsName + " / " + m_fsName);
        if (bgfx::isValid(vsh)) bgfx::destroy(vsh);
        if (bgfx::isValid(fsh)) bgfx::destroy(fsh);
        return false;
    }

    bgfx::ProgramHandle newProgram = bgfx::createProgram(vsh, fsh, /*destroyShaders=*/true);
    if (!bgfx::isValid(newProgram))
    {
        Logger::Log("Shader::Reload — bgfx::createProgram failed, keeping old program.");
        return false;
    }

    // Destroy old resources
    for (auto& [n, u] : m_uniforms)
        if (bgfx::isValid(u.handle)) bgfx::destroy(u.handle);

    if (bgfx::isValid(m_program))
        bgfx::destroy(m_program);

    // Re-reflect (handles may have changed, array sizes may differ)
    // We need fresh handles for reflection before destroyShaders consumed them,
    // so load again just for reflection then destroy those copies.
    {
        bgfx::ShaderHandle vsRef = LoadShaderBinary(m_vsName);
        bgfx::ShaderHandle fsRef = LoadShaderBinary(m_fsName);
        ReflectUniforms(vsRef, fsRef);
        if (bgfx::isValid(vsRef)) bgfx::destroy(vsRef);
        if (bgfx::isValid(fsRef)) bgfx::destroy(fsRef);
    }

    m_program = newProgram;
    textureBindings = ParseAllTextureBindings();

    Logger::Log("Shader::Reload succeeded for: " + m_vsName + " / " + m_fsName);
    return true;
}

// ---------------------------------------------------------------------------
// Uniform lookup
// ---------------------------------------------------------------------------
const UniformMeta* Shader::FindUniform(const std::string& uname) const
{
    auto it = m_uniforms.find(uname);
    if (it != m_uniforms.end())
        return &it->second;

    if (!AllowMissingUniforms)
        Logger::Log("Shader warning: uniform \"" + uname + "\" not found in " + m_vsName);

    return nullptr;
}

// ---------------------------------------------------------------------------
// Scalar / small-type setters  (all map to Vec4 with padding)
// ---------------------------------------------------------------------------
void Shader::SetUniform(const std::string& uname, int value)
{
    SetUniform(uname, static_cast<float>(value));
}

void Shader::SetUniform(const std::string& uname, bool value)
{
    SetUniform(uname, value ? 1.0f : 0.0f);
}

void Shader::SetUniform(const std::string& uname, float value)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    float data[4] = { value, 0.0f, 0.0f, 0.0f };
    bgfx::setUniform(u->handle, data);
}

void Shader::SetUniform(const std::string& uname, const glm::vec2& value)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    float data[4] = { value.x, value.y, 0.0f, 0.0f };
    bgfx::setUniform(u->handle, data);
}

void Shader::SetUniform(const std::string& uname, const glm::vec3& value)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    float data[4] = { value.x, value.y, value.z, 0.0f };
    bgfx::setUniform(u->handle, data);
}

void Shader::SetUniform(const std::string& uname, const glm::vec4& value)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    bgfx::setUniform(u->handle, glm::value_ptr(value));
}

// mat2 → pack column-major into one Vec4: [m00, m10, m01, m11]
void Shader::SetUniform(const std::string& uname, const glm::mat2& value)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    float data[4] = {
        value[0][0], value[0][1],
        value[1][0], value[1][1]
    };
    bgfx::setUniform(u->handle, data);
}

void Shader::SetUniform(const std::string& uname, const glm::mat3& value)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    // bgfx Mat3 layout: 3 Vec4 rows, column-major, last component of each row is padding.
    float data[12];
    const float* src = glm::value_ptr(value); // column-major: col0[3], col1[3], col2[3]
    // Transpose to row-major for bgfx Mat3 (row0, row1, row2, each padded to vec4)
    data[0] = src[0]; data[1] = src[3]; data[2] = src[6]; data[3] = 0.0f; // row 0
    data[4] = src[1]; data[5] = src[4]; data[6] = src[7]; data[7] = 0.0f; // row 1
    data[8] = src[2]; data[9] = src[5]; data[10] = src[8]; data[11] = 0.0f; // row 2
    bgfx::setUniform(u->handle, data);
}

void Shader::SetUniform(const std::string& uname, const glm::mat4& value)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    bgfx::setUniform(u->handle, glm::value_ptr(value));
}

// ---------------------------------------------------------------------------
// Array setters — each element is padded to a Vec4 slot
// ---------------------------------------------------------------------------
void Shader::SetUniform(const std::string& uname, const std::vector<float>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    std::vector<float> padded(values.size() * 4, 0.0f);
    for (size_t i = 0; i < values.size(); ++i)
        padded[i * 4] = values[i];

    bgfx::setUniform(u->handle, padded.data(), uint16_t(values.size()));
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec2>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    std::vector<float> padded(values.size() * 4, 0.0f);
    for (size_t i = 0; i < values.size(); ++i)
    {
        padded[i * 4 + 0] = values[i].x;
        padded[i * 4 + 1] = values[i].y;
    }

    bgfx::setUniform(u->handle, padded.data(), uint16_t(values.size()));
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec3>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    std::vector<float> padded(values.size() * 4, 0.0f);
    for (size_t i = 0; i < values.size(); ++i)
    {
        padded[i * 4 + 0] = values[i].x;
        padded[i * 4 + 1] = values[i].y;
        padded[i * 4 + 2] = values[i].z;
    }

    bgfx::setUniform(u->handle, padded.data(), uint16_t(values.size()));
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec4>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    bgfx::setUniform(u->handle, glm::value_ptr(values[0]), uint16_t(values.size()));
}

// mat2 array: each mat2 → one Vec4 slot
void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat2>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    std::vector<float> packed(values.size() * 4);
    for (size_t i = 0; i < values.size(); ++i)
    {
        packed[i * 4 + 0] = values[i][0][0];
        packed[i * 4 + 1] = values[i][0][1];
        packed[i * 4 + 2] = values[i][1][0];
        packed[i * 4 + 3] = values[i][1][1];
    }

    bgfx::setUniform(u->handle, packed.data(), uint16_t(values.size()));
}

// mat3 array: each mat3 → 3 Vec4 rows (12 floats)
void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat3>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    std::vector<float> packed(values.size() * 12, 0.0f);
    for (size_t i = 0; i < values.size(); ++i)
    {
        const float* src = glm::value_ptr(values[i]);
        float* dst = packed.data() + i * 12;
        // row 0
        dst[0] = src[0]; dst[1] = src[3]; dst[2] = src[6]; dst[3] = 0.0f;
        // row 1
        dst[4] = src[1]; dst[5] = src[4]; dst[6] = src[7]; dst[7] = 0.0f;
        // row 2
        dst[8] = src[2]; dst[9] = src[5]; dst[10] = src[8]; dst[11] = 0.0f;
    }

    bgfx::setUniform(u->handle, packed.data(), uint16_t(values.size()));
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat4>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    bgfx::setUniform(u->handle, glm::value_ptr(values[0]), uint16_t(values.size()));
}

// ---------------------------------------------------------------------------
// Texture setters
// ---------------------------------------------------------------------------
void Shader::SetTexture(const std::string& uname, bgfx::TextureHandle texture)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    bgfx::setTexture(u->samplerSlot, u->handle, texture);
}

void Shader::SetTexture(const hashed_string& uname, Texture* texture)
{
    const UniformMeta* u = FindUniform(uname.str());
    if (!u) return;

    bgfx::TextureHandle handle =
        (texture && texture->valid) ? texture->getHandle() : s_missingTexture;

    bgfx::setTexture(u->samplerSlot, u->handle, handle);
}

void Shader::SetCubemapTexture(const std::string& uname, bgfx::TextureHandle texture)
{
    // In bgfx the sampler type is opaque; cube vs 2D is encoded in the texture handle.
    // The API call is identical to SetTexture.
    SetTexture(uname, texture);
}

// ---------------------------------------------------------------------------
// Missing texture fallback
// ---------------------------------------------------------------------------
void Shader::EnsureMissingTexture()
{
    if (bgfx::isValid(s_missingTexture))
        return;

    // 2×2 magenta checkerboard as a placeholder
    const uint32_t M = 0xFFFF00FF; // RGBA magenta
    const uint32_t B = 0xFF000000; // RGBA black
    uint32_t pixels[4] = { M, B, B, M };

    const bgfx::Memory* mem = bgfx::copy(pixels, sizeof(pixels));
    s_missingTexture = bgfx::createTexture2D(
        2, 2,
        /*hasMips=*/false,
        /*numLayers=*/1,
        bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
        mem
    );
    bgfx::setName(s_missingTexture, "missing_texture");
}

// ---------------------------------------------------------------------------
// UseProgram / Submit
// ---------------------------------------------------------------------------
void Shader::UseProgram()
{
    EnsureMissingTexture();
    ApplyTextureBindings();
}

void Shader::Submit(uint16_t viewId) const
{
    bgfx::submit(viewId, m_program);
}

// ---------------------------------------------------------------------------
// @texture annotation parsing
// ---------------------------------------------------------------------------

// Parse a single source string for:
//   uniform sampler2D  u_name; // @texture path/to/file.png
//   uniform samplerCube u_env; // @texture path/to/cubemap
std::unordered_map<hashed_string, std::string>
Shader::ParseTextureBindings(const std::string& sourceCode)
{
    std::unordered_map<hashed_string, std::string> result;

    std::regex re(R"(uniform\s+(?:sampler2D|samplerCube)\s+(\w+)\s*;.*@texture\s+([^\s]+))");
    std::smatch match;
    auto it = sourceCode.cbegin();

    while (std::regex_search(it, sourceCode.cend(), match, re))
    {
        result[match[1].str()] = match[2].str();
        it = match.suffix().first;
    }

    return result;
}

// Read both .sh source files and merge their @texture annotations.
std::unordered_map<hashed_string, std::string>
Shader::ParseAllTextureBindings() const
{
    std::unordered_map<hashed_string, std::string> result;

    for (const std::string& path : { m_vsSourcePath, m_fsSourcePath })
    {
        if (path.empty()) continue;

        const std::string source = FileSystemEngine::ReadFile(path);
        if (source.empty()) continue;

        auto parsed = ParseTextureBindings(source);
        for (auto& [k, v] : parsed)
            result[k] = v;   // later stage wins on collision
    }

    return result;
}

// Apply all auto-bindings: resolve each path via AssetRegistry, call SetTexture.
void Shader::ApplyTextureBindings()
{
    for (const auto& [uniformName, path] : textureBindings)
    {
        Texture* tex = AssetRegistry::GetTextureFromFile(path);
        SetTexture(uniformName, tex);
    }
}