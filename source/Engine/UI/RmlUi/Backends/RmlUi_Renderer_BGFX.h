/*
 * RmlUi bgfx Render Interface
 *
 * Full port of the OpenGL 3 render interface to bgfx.
 * Supports layers, filters, clip masks (stencil), scissor, transforms,
 * and all compositing/blend modes required by RmlUi 6.x.
 *
 * Requires: bgfx, bx, Shader.hpp, ViewIdManager.h
 */

#ifndef RMLUI_BACKENDS_RENDERER_BGFX_H
#define RMLUI_BACKENDS_RENDERER_BGFX_H

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Types.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <memory>

class Shader;

// ── Program identifiers ─────────────────────────────────────────────────────
enum class RmlProgramId : int
{
    None = 0,
    Color,           // vertex-color only
    Texture,         // textured + vertex-color (premultiplied alpha)
    Passthrough,     // fullscreen blit / layer composite
    BlendMask,       // composite with blend-mask texture
    Blur,            // separable Gaussian blur
    DropShadow,      // drop-shadow filter
    ColorMatrix,     // generic 4×5 colour-matrix filter
    Creation,        // stencil-write (clip-mask geometry)
    Gradient,        // linear / radial / conic gradient shader
    Count
};

// ── Compiled-filter storage ─────────────────────────────────────────────────
enum class FilterType { None, Blur, DropShadow, ColorMatrix, MaskImage };

struct CompiledFilter
{
    FilterType type = FilterType::None;
    // Blur / DropShadow
    float sigma = 0.f;
    // DropShadow extras
    Rml::Vector2f offset = {0, 0};
    Rml::Colourf  color  = {0, 0, 0, 1};
    // ColorMatrix (row-major 4×5, stored as mat4 + vec4 translate)
    float color_matrix[16] = {};
    float color_translate[4] = {};
    // MaskImage
    bgfx::TextureHandle mask_texture = BGFX_INVALID_HANDLE;
};

// ── Compiled-shader (decorators) storage ────────────────────────────────────
enum class ShaderType { None, Gradient };

struct ShaderGradientStop { float position; Rml::Colourf color; };

struct CompiledShaderData
{
    ShaderType type = ShaderType::None;
    // Gradient
    int   gradient_function = 0; // 0=linear, 1=radial, 2=conic, 3=repeating-linear, …
    Rml::Vector2f p  = {};
    Rml::Vector2f q  = {};
    std::vector<ShaderGradientStop> stops;
    bgfx::TextureHandle stop_texture = BGFX_INVALID_HANDLE;
};

// ── Framebuffer wrapper ─────────────────────────────────────────────────────
struct BgfxFramebuffer
{
    bgfx::FrameBufferHandle fb   = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle     color = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle     depth_stencil = BGFX_INVALID_HANDLE;
    int width  = 0;
    int height = 0;
};

// ── Compiled geometry ───────────────────────────────────────────────────────
struct BgfxCompiledGeometry
{
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE;
    int num_indices = 0;
};

// ═════════════════════════════════════════════════════════════════════════════
//  RenderInterface_BGFX
// ═════════════════════════════════════════════════════════════════════════════
class RenderInterface_BGFX : public Rml::RenderInterface
{
public:
    RenderInterface_BGFX();
    ~RenderInterface_BGFX();

    // Returns true if all programs loaded successfully.
    explicit operator bool() const { return m_valid; }

    // The viewport should be updated whenever the window size changes.
    void SetViewport(int viewport_width, int viewport_height,
                     int viewport_offset_x = 0, int viewport_offset_y = 0);

    // Sets up bgfx states for taking rendering commands from RmlUi.
    void BeginFrame();
    // Submits all remaining draws and finalises the frame.
    void EndFrame();

    // Clears the current render target.
    void Clear();

    // ── Rml::RenderInterface overrides ──────────────────────────────────────

    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
                                                 Rml::Span<const int> indices) override;
    void RenderGeometry(Rml::CompiledGeometryHandle handle,
                        Rml::Vector2f translation,
                        Rml::TextureHandle texture) override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle handle) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions,
                                    const Rml::String& source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source_data,
                                        Rml::Vector2i source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture_handle) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

    void EnableClipMask(bool enable) override;
    void RenderToClipMask(Rml::ClipMaskOperation mask_operation,
                          Rml::CompiledGeometryHandle geometry,
                          Rml::Vector2f translation) override;

    void SetTransform(const Rml::Matrix4f* transform) override;

    Rml::LayerHandle PushLayer() override;
    void CompositeLayers(Rml::LayerHandle source, Rml::LayerHandle destination,
                         Rml::BlendMode blend_mode,
                         Rml::Span<const Rml::CompiledFilterHandle> filters) override;
    void PopLayer() override;

    Rml::TextureHandle SaveLayerAsTexture() override;

    Rml::CompiledFilterHandle SaveLayerAsMaskImage() override;

    Rml::CompiledFilterHandle CompileFilter(const Rml::String& name,
                                             const Rml::Dictionary& parameters) override;
    void ReleaseFilter(Rml::CompiledFilterHandle filter) override;

    Rml::CompiledShaderHandle CompileShader(const Rml::String& name,
                                             const Rml::Dictionary& parameters) override;
    void RenderShader(Rml::CompiledShaderHandle shader_handle,
                      Rml::CompiledGeometryHandle geometry_handle,
                      Rml::Vector2f translation,
                      Rml::TextureHandle texture) override;
    void ReleaseShader(Rml::CompiledShaderHandle effect_handle) override;

    // Special texture-handle sentinels (same semantics as GL3)
    static constexpr Rml::TextureHandle TextureEnableWithoutBinding = Rml::TextureHandle(-1);
    static constexpr Rml::TextureHandle TexturePostprocess          = Rml::TextureHandle(-2);

    // ── Utility ─────────────────────────────────────────────────────────────
    const Rml::Matrix4f& GetTransform() const { return m_transform; }

private:
    // ── Internal helpers ────────────────────────────────────────────────────
    bool LoadPrograms();
    void DestroyPrograms();

    bgfx::ViewId AllocateView();                       // get next view from ViewIdManager
    void SetupView(bgfx::ViewId view, const BgfxFramebuffer& fb);
    void SetupViewRect(bgfx::ViewId view);
    void EnsureView();                                  // make sure m_currentView is valid

    void UseProgram(RmlProgramId id);

    void SubmitGeometry(Rml::CompiledGeometryHandle handle,
                        Rml::Vector2f translation,
                        Rml::TextureHandle texture,
                        RmlProgramId program_override = RmlProgramId::None);

    void DrawFullscreenQuad(bgfx::TextureHandle texture, RmlProgramId program);
    void DrawFullscreenQuad(bgfx::TextureHandle texture, RmlProgramId program,
                            Rml::Vector2f uv_offset, Rml::Vector2f uv_scaling);

    void BlitLayerToPostprocessPrimary(Rml::LayerHandle layer_handle);
    void RenderFilters(Rml::Span<const Rml::CompiledFilterHandle> filter_handles);
    void RenderBlur(float sigma, BgfxFramebuffer& source_dest, BgfxFramebuffer& temp,
                    Rml::Rectanglei window_flipped);

    uint64_t BuildBaseState() const;
    uint32_t BuildStencilState() const;

    Rml::Matrix4f MakeProjection() const;
    void SetTransformUniform(Rml::Vector2f translation);

    // ── Framebuffer helpers ─────────────────────────────────────────────────
    BgfxFramebuffer CreateFramebuffer(int w, int h, bool with_depth_stencil = true);
    void DestroyFramebuffer(BgfxFramebuffer& fb);

    // ── Layer stack ─────────────────────────────────────────────────────────
    class RenderLayerStack
    {
    public:
        RenderLayerStack() = default;
        ~RenderLayerStack();

        Rml::LayerHandle PushLayer(RenderInterface_BGFX& ri);
        void PopLayer();

        const BgfxFramebuffer& GetLayer(Rml::LayerHandle h) const;
        const BgfxFramebuffer& GetTopLayer() const;
        Rml::LayerHandle       GetTopLayerHandle() const;

        BgfxFramebuffer& GetPostprocessPrimary();
        BgfxFramebuffer& GetPostprocessSecondary();
        BgfxFramebuffer& GetPostprocessTertiary();
        BgfxFramebuffer& GetBlendMask();

        void SwapPostprocessPrimarySecondary();

        void BeginFrame(RenderInterface_BGFX& ri, int w, int h);
        void EndFrame();

    private:
        void DestroyAll(RenderInterface_BGFX& ri);
        BgfxFramebuffer& EnsurePostprocess(RenderInterface_BGFX& ri, int idx);

        int width_  = 0;
        int height_ = 0;
        int layers_size_ = 0;

        std::vector<BgfxFramebuffer> fb_layers_;
        std::vector<BgfxFramebuffer> fb_postprocess_;

        RenderInterface_BGFX* ri_ = nullptr; // non-owning, for cleanup
    };

    RenderLayerStack m_layers;

    // ── Programs ────────────────────────────────────────────────────────────
    Shader* m_programs[static_cast<int>(RmlProgramId::Count)] = {};
    RmlProgramId m_activeProgram = RmlProgramId::None;

    // ── Uniforms (created once, reused) ─────────────────────────────────────
    bgfx::UniformHandle u_transform       = BGFX_INVALID_HANDLE; // mat4
    bgfx::UniformHandle u_translate       = BGFX_INVALID_HANDLE; // vec4
    bgfx::UniformHandle u_texParams       = BGFX_INVALID_HANDLE; // vec4 (uv_offset.xy, uv_scale.xy)
    bgfx::UniformHandle u_blurParams      = BGFX_INVALID_HANDLE; // vec4 (sigma, dir_x, dir_y, 0)
    bgfx::UniformHandle u_texelSize       = BGFX_INVALID_HANDLE; // vec4 (1/w, 1/h, 0, 0)
    bgfx::UniformHandle u_colorMatrix     = BGFX_INVALID_HANDLE; // mat4
    bgfx::UniformHandle u_colorTranslate  = BGFX_INVALID_HANDLE; // vec4
    bgfx::UniformHandle u_shadowExtra     = BGFX_INVALID_HANDLE; // vec4 (offset.xy, 0, 0)
    bgfx::UniformHandle u_shadowColor     = BGFX_INVALID_HANDLE; // vec4
    bgfx::UniformHandle u_gradientParams  = BGFX_INVALID_HANDLE; // vec4 (func, num_stops, 0, 0)
    bgfx::UniformHandle u_gradientP       = BGFX_INVALID_HANDLE; // vec4 (p.xy, q.xy)
    bgfx::UniformHandle s_texture0        = BGFX_INVALID_HANDLE; // sampler
    bgfx::UniformHandle s_texture1        = BGFX_INVALID_HANDLE; // sampler (blend mask / stops)

    // ── Vertex layout ───────────────────────────────────────────────────────
    bgfx::VertexLayout m_vertexLayout;

    // ── Geometry registry ───────────────────────────────────────────────────
    Rml::CompiledGeometryHandle m_nextGeometryId = 1;
    std::unordered_map<Rml::CompiledGeometryHandle, BgfxCompiledGeometry> m_geometries;

    // ── Filter / shader registries ──────────────────────────────────────────
    Rml::CompiledFilterHandle m_nextFilterId = 1;
    std::unordered_map<Rml::CompiledFilterHandle, CompiledFilter> m_filters;

    Rml::CompiledShaderHandle m_nextShaderId = 1;
    std::unordered_map<Rml::CompiledShaderHandle, CompiledShaderData> m_shaders;

    // ── Texture bookkeeping ─────────────────────────────────────────────────
    Rml::TextureHandle m_nextTextureId = 1;

    // ── Fullscreen-quad geometry (pre-built) ────────────────────────────────
    Rml::CompiledGeometryHandle m_fullscreenQuad = {};

    // ── Render state ────────────────────────────────────────────────────────
    Rml::Matrix4f m_transform  = Rml::Matrix4f::Identity();
    Rml::Matrix4f m_projection = Rml::Matrix4f::Identity();

    bool m_scissorEnabled = false;
    Rml::Rectanglei m_scissorRegion = {};

    bool m_clipMaskEnabled = false;
    uint8_t m_stencilRef   = 0;      // current stencil reference value
    uint8_t m_stencilValue = 1;      // value written during clip-mask

    bgfx::ViewId m_currentView = 0;
    bgfx::ViewId m_baseView    = 0;  // first view allocated this frame
    bool         m_viewDirty   = true;

    bgfx::TextureHandle m_lastBoundTexture = BGFX_INVALID_HANDLE;

    int m_viewportWidth   = 0;
    int m_viewportHeight  = 0;
    int m_viewportOffsetX = 0;
    int m_viewportOffsetY = 0;

    bool m_valid = false;

    uint32_t m_drawOrder = 0;   // sequential ordering within a view
};

#endif // RMLUI_BACKENDS_RENDERER_BGFX_H
