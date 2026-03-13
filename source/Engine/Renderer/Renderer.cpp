// Renderer.cpp — bgfx port
#include "Renderer.h"

#include "../EngineMain.h"
#include "../LightSystem/LightManager.h"
#include "../DebugDraw.hpp"
#include "../FogManager.h"

#include "Abstractions/ViewIdManager.h"

#include <bgfx/bgfx.h>
#include <BgfxStateManager.h>

// -----------------------------------------------------------------------
// bgfx clear-color helper  (RGBA packed as uint32_t 0xRRGGBBAA)
// -----------------------------------------------------------------------
static constexpr uint32_t kClearBlack = 0x000000ff;
static constexpr uint32_t kClearAlpha1 = 0x000000ff;

// -----------------------------------------------------------------------
// Constructor / destructor
// -----------------------------------------------------------------------
Renderer::Renderer()
{
    ivec2 screenResolution = GetScreenResolution();

    InitFrameBuffers();
    InitResolveFrameBuffers();

    fullscreenShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_postprocessing");
    blurShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_motionBlur");
    blurApplyShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_motionBlur_apply");

    BlurResultBuffer = new RenderTexture(screenResolution.x, screenResolution.y,
        TextureFormat::RGBA16F);
    BlurResultBuffer->SetName("BlurResultBuffer");

    BlurAccumulatedBuffer = new RenderTexture(screenResolution.x, screenResolution.y,
        TextureFormat::RGBA16F);
    BlurAccumulatedBuffer->SetName("BlurAccumulatedBuffer");

    if (LightManager::DirectionalShadowsEnabled)
    {
        DirectionalShadowMap = new RenderTexture(
            LightManager::ShadowMapResolution, LightManager::ShadowMapResolution,
            TextureFormat::Depth32F, TextureType::Texture2D);
        DirectionalShadowMap->SetName("DirectionalShadowMap");
        DirectionalShadowMapFBO = new Framebuffer();
        DirectionalShadowMapFBO->attachDepth(DirectionalShadowMap);

        DetailDirectionalShadowMap = new RenderTexture(
            LightManager::ShadowMapResolution, LightManager::ShadowMapResolution,
            TextureFormat::Depth32F, TextureType::Texture2D);
        DetailDirectionalShadowMap->SetName("DetailDirectionalShadowMap");
        DetailDirectionalShadowMapFBO = new Framebuffer();
        DetailDirectionalShadowMapFBO->attachDepth(DetailDirectionalShadowMap);
    }

    InitFullscreenBuffers();
}

Renderer::~Renderer()
{
    delete colorBuffer;
    delete depthBuffer;

    if (bgfx::isValid(m_fullscreenVB))
        bgfx::destroy(m_fullscreenVB);
}

// -----------------------------------------------------------------------
// RenderLevel
// -----------------------------------------------------------------------
void Renderer::RenderLevel(Level* level)
{
    if (LightManager::DirectionalShadowsEnabled)
    {
        RenderDirectionalLightShadows(level->ShadowRenderList, *DirectionalShadowMapFBO, 4);
        RenderDirectionalLightShadows(level->DetailShadowRenderList, *DetailDirectionalShadowMapFBO, 3);
    }

    for (auto drawable : level->VissibleRenderList)
        drawable->PreDraw();

    RenderCameraForward(level->VissibleRenderList);

    ivec2 screenResolution = GetScreenResolution();
    BlurAccumulatedBuffer->resize(screenResolution.x, screenResolution.y);
    BlurResultBuffer->resize(screenResolution.x, screenResolution.y);

    const bool blurEnabled = false;

    if (blurEnabled)
    {
        // ---- Motion blur accumulate pass ----
        BlurResultBuffer->setAsRenderTarget();

        bgfx::setViewClear(ViewIdManager::GetCurrentId(),
            BGFX_CLEAR_COLOR, kClearBlack, 1.0f, 0);

        blurShader->UseProgram();
        blurShader->SetTexture("uAccumulated", BlurAccumulatedBuffer->textureHandle());
        blurShader->SetTexture("uCustomIdTex", customIdResolveBuffer->textureHandle());
        blurShader->SetUniform("uDeltaTime", EngineMain::MainInstance->Paused
            ? 0.0f
            : Time::DeltaTimeFNoTimeScale);
        blurShader->SetUniform("GameTime", (float)Time::GameTime);
        blurShader->SetUniform("uPersistence", 0.20f);
        blurShader->SetUniform("uMotionScale", 3.0f);
        blurShader->SetTexture("screenTexture", colorResolveBuffer->textureHandle());

        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::Apply();
        RenderFullscreenQuad(blurShader);

        BlurAccumulatedBuffer->copyFrom(BlurResultBuffer);

        // ---- Motion blur apply pass ----
        BlurResultBuffer->setAsRenderTarget();
        blurApplyShader->UseProgram();
        blurApplyShader->SetTexture("screenTexture", colorResolveBuffer->textureHandle());
        blurApplyShader->SetTexture("blurTexture", BlurAccumulatedBuffer->textureHandle());

        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::Apply();
        RenderFullscreenQuad(blurApplyShader);
    }

    // ---- Final blit to backbuffer ----
    // FIX: Use m_finalBlitViewId (a high view ID, allocated after all FBO views)
    // so bgfx executes this AFTER the scene FBO passes, not before them.
    // View 0 executes first in bgfx's default order, so we must NOT use view 0
    // for a pass that depends on FBO results.
    bgfx::TextureHandle resultTex = blurEnabled
        ? BlurResultBuffer->textureHandle()
        : colorResolveBuffer->textureHandle();

#if __EMSCRIPTEN__
	resultTex = colorBuffer->textureHandle();
#endif // __EMSCRIPTEN__


    ivec2 nativeRes = GetNativeScreenResolution();

    ViewIdManager::GiveNextId();

    bgfx::setViewRect(ViewIdManager::GetCurrentId(), 0, 0,
        static_cast<uint16_t>(nativeRes.x),
        static_cast<uint16_t>(nativeRes.y));
    bgfx::setViewFrameBuffer(ViewIdManager::GetCurrentId(), BGFX_INVALID_HANDLE);


    fullscreenShader->UseProgram();
    fullscreenShader->SetTexture("screenTexture", resultTex);
    fullscreenShader->SetUniform("screenResolution", nativeRes);


    BgfxStateManager::Reset();
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
    BgfxStateManager::Apply();
    RenderFullscreenQuad(fullscreenShader);
}

// -----------------------------------------------------------------------
// RenderCameraForward
// -----------------------------------------------------------------------
void Renderer::RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList)
{
    ivec2 res = GetScreenResolution();

    // ---- MSAA bookkeeping ----
#if defined(BGFX_PLATFORM_EMSCRIPTEN)
    MultiSampleCount = 0;
#endif

    if (MultiSampleCount)
    {
        if (colorBuffer->type() == TextureType::Texture2D)
            InitFrameBuffers();

        colorBuffer->setSamples(MultiSampleCount);
        depthBuffer->setSamples(MultiSampleCount);
    }
    else
    {
        if (colorBuffer->type() == TextureType::Texture2DMultisample)
            InitFrameBuffers();

        colorBuffer->setSamples(0);
        depthBuffer->setSamples(0);
    }

    // ---- Resize all buffers ----
    // If any texture is resized its handle changes, so we must rebuild the
    // FBOs that reference it immediately after resizing.
    bool resized = false;
    resized |= colorBuffer->resize(res.x, res.y);
    resized |= depthBuffer->resize(res.x, res.y);
    if (resized)
    {
        // Texture handles changed — rebuild both FBOs that reference them.
        //forwardDepthFBO->attachDepth(depthBuffer);
        forwardFBO->attachColor(colorBuffer, 0u);
        forwardFBO->attachDepth(depthBuffer);
    }

    bool resolveResized = false;
    resolveResized |= colorResolveBuffer->resize(res.x, res.y);
    resolveResized |= customIdResolveBuffer->resize(res.x, res.y);
    resolveResized |= depthResolveBuffer->resize(res.x, res.y);
    if (resolveResized)
    {
        forwardResolveFBO->attachColor(colorResolveBuffer, 0u);
        forwardResolveFBO->attachDepth(depthResolveBuffer);
        customIdFBO->attachColor(customIdResolveBuffer, 0u);
    }

    // ====================================================================
    // Pass A — Depth pre-pass
    // Uses forwardDepthFBO — its own dedicated view ID, separate from
    // forwardFBO. This is critical: if both passes share a view, the second
    // setViewClear call overwrites the first (depth clear is lost) and bgfx
    // may reorder draws within the view, breaking the pre-pass.
    // ====================================================================
    {

        forwardFBO->bind(0, 0,
            static_cast<uint16_t>(res.x),
            static_cast<uint16_t>(res.y));

        bgfx::ViewId vid = ViewIdManager::GetCurrentId();


        // Clear depth only — no color attachment on this FBO.
        bgfx::setViewClear(vid, BGFX_CLEAR_DEPTH, kClearBlack, 1.0f, 0);

        // Depth write only, less-than test, back-face cull.
        BgfxStateManager::Reset();
        BgfxStateManager::SetWriteRGB(false);
        BgfxStateManager::SetWriteAlpha(false);
        BgfxStateManager::SetWriteDepth(true);
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);

        for (auto* mesh : VissibleRenderList)
        {
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawDepth(Camera::finalizedView, P);
        }
    }

    // Resolve depth pre-pass → single-sample. Safe here because we haven't
    // written any color yet and forwardDepthFBO has no color attachment.
    forwardFBO->resolveDepthOnly(*forwardResolveFBO);

    // ====================================================================
    // Pass B — Opaque + transparent color pass
    // Uses forwardFBO — a different view ID from Pass A. This guarantees
    // the GPU executes Pass A fully before Pass B begins.
    // ====================================================================
    {

        forwardFBO->bind(0, 0,
            static_cast<uint16_t>(res.x),
            static_cast<uint16_t>(res.y));

        bgfx::ViewId vid = ViewIdManager::GetCurrentId();


        // Clear color only — depth was already written by Pass A.
        //bgfx::setViewClear(vid, BGFX_CLEAR_COLOR, kClearBlack, 1.0f, 0);

        // Opaque: RGB+A write, no depth write, lequal test, back-face cull.
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::LEqual);
        BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);

        for (auto* mesh : VissibleRenderList)
        {
            if (mesh->Transparent) continue;
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawForward(Camera::finalizedView, P);
        }

        // Transparent: RGB+A write, no depth write, lequal test, alpha blend.
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::LEqual);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Alpha);
        BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);

        Level::Current->BspData.RenderTransparentFaces();

        for (auto* mesh : VissibleRenderList)
        {
            if (!mesh->Transparent) continue;
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawForward(Camera::finalizedView, P);
        }

        DebugDraw::Draw();
    }

    // Resolve color + depth → single-sample resolve FBO.
    forwardFBO->resolve(*forwardResolveFBO);

    // ====================================================================
    // Pass C — Custom ID pass  (single-sample, no depth write)
    // ====================================================================
    {

        customIdFBO->bind(0, 0,
            static_cast<uint16_t>(res.x),
            static_cast<uint16_t>(res.y));

        bgfx::ViewId vid = ViewIdManager::GetCurrentId();


        bgfx::setViewClear(vid, BGFX_CLEAR_COLOR, kClearBlack, 1.0f, 0);

        // No depth test — write color IDs only.
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);

        for (auto* mesh : VissibleRenderList)
        {
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawCustomId(Camera::finalizedView, P);
        }
    }
}

// -----------------------------------------------------------------------
// RenderDirectionalLightShadows
// -----------------------------------------------------------------------
void Renderer::RenderDirectionalLightShadows(vector<IDrawMesh*>& ShadowRenderList,
    Framebuffer& fbo,
    int                 /*numCascades*/)
{

}

// -----------------------------------------------------------------------
// RenderFullscreenQuad
// State must be set by the caller via BgfxStateManager before calling this.
// -----------------------------------------------------------------------
void Renderer::RenderFullscreenQuad(Shader* shader)
{
    bgfx::setVertexBuffer(0, m_fullscreenVB);
	shader->Submit(ViewIdManager::GetCurrentId());
}

// -----------------------------------------------------------------------
// SetSurfaceShaderUniforms
// -----------------------------------------------------------------------
void Renderer::SetSurfaceShaderUniforms(Shader* shader, float brightnessScale)
{
    if (!shader) return;

    shader->SetUniform("lightDirection", LightManager::LightDirection);
    shader->SetUniform("brightness", 1.0f * brightnessScale);

    if (LightManager::DirectionalShadowsEnabled)
    {
        shader->SetUniform("lightMatrix1",
            LightManager::lightProjection1 * LightManager::lightView1);
        shader->SetUniform("lightMatrix2",
            LightManager::lightProjection2 * LightManager::lightView2);
        shader->SetUniform("lightMatrix3",
            LightManager::lightProjection3 * LightManager::lightView3);
        shader->SetUniform("lightMatrix4",
            LightManager::lightProjection4 * LightManager::lightView4);

        shader->SetTexture("shadowMap",
            EngineMain::MainInstance->MainRenderer->DirectionalShadowMap->textureHandle());
        shader->SetTexture("shadowMapDetail",
            EngineMain::MainInstance->MainRenderer->DetailDirectionalShadowMap->textureHandle());
        shader->SetTexture("shadowMapRaw",
            EngineMain::MainInstance->MainRenderer->DirectionalShadowMap->textureHandle());
        shader->SetTexture("shadowMapDetailRaw",
            EngineMain::MainInstance->MainRenderer->DetailDirectionalShadowMap->textureHandle());

        shader->SetUniform("shadowDistance1", LightManager::LightDistance1);
        shader->SetUniform("shadowDistance2", LightManager::LightDistance2);
        shader->SetUniform("shadowDistance3", LightManager::LightDistance3);
        shader->SetUniform("shadowDistance4", LightManager::LightDistance4);

        shader->SetUniform("shadowRadius1", LightManager::LightRadius1);
        shader->SetUniform("shadowRadius2", LightManager::LightRadius2);
        shader->SetUniform("shadowRadius3", LightManager::LightRadius3);
        shader->SetUniform("shadowRadius4", LightManager::LightRadius4);
    }

    shader->SetTexture("depthTexture",
        EngineMain::MainInstance->MainRenderer->depthResolveBuffer->textureHandle());

    shader->SetUniform("fog_start", FogManager::StartDistance);
    shader->SetUniform("fog_end", FogManager::EndDistance);
    shader->SetUniform("fog_opacity", FogManager::Opacity);
    shader->SetUniform("fog_color", FogManager::Color);

    shader->SetUniform("cameraPosition", Camera::finalizedPosition);
    shader->SetUniform("shadowMapSize", LightManager::ShadowMapResolution);
    shader->SetUniform("shaddowOffsetScale", LightManager::ShaddowOffsetScale);
}

// -----------------------------------------------------------------------
// Resolution helpers
// -----------------------------------------------------------------------
inline ivec2 Renderer::GetScreenResolution() const
{
    return ivec2(EngineMain::MainInstance->ScreenSize.x * ResolutionScale,
        EngineMain::MainInstance->ScreenSize.y * ResolutionScale);
}

inline ivec2 Renderer::GetNativeScreenResolution() const
{
    return ivec2(EngineMain::MainInstance->ScreenSize.x,
        EngineMain::MainInstance->ScreenSize.y);
}

// -----------------------------------------------------------------------
// InitFullscreenBuffers
// -----------------------------------------------------------------------
void Renderer::InitFullscreenBuffers()
{
    m_fullscreenLayout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    // Single oversized triangle covering the entire NDC clip space.
    // bgfx default state is TRIANGLE_LIST, so the old 4-vertex strip
    // only drew one triangle (verts 0-1-2; vert 3 was ignored).
    // A 3-vertex oversized triangle needs no BGFX_STATE_PT_TRISTRIP,
    // no index buffer, and has no diagonal seam.
    //
    //   (-1, 3)
    //     |\
    //     |  \         clips to the [-1,+1] viewport on the GPU
    //     |    \
    //  (-1,-1)---(3,-1)
    //
    struct FullscreenVertex { float x, y, u, v; };
    static const FullscreenVertex kVerts[3] = {
        { -1.0f,  3.0f,   0.0f,  2.0f },   // top-left  (oversized)
        { -1.0f, -1.0f,   0.0f,  0.0f },   // bottom-left
        {  3.0f, -1.0f,   2.0f,  0.0f },   // bottom-right (oversized)
    };

    m_fullscreenVB = bgfx::createVertexBuffer(
        bgfx::makeRef(kVerts, sizeof(kVerts)),
        m_fullscreenLayout
    );
}

// -----------------------------------------------------------------------
// InitFrameBuffers
// -----------------------------------------------------------------------
void Renderer::InitFrameBuffers()
{
    ivec2 screenResolution = GetScreenResolution();
    TextureFormat colorFmt = TextureFormat::RGBA16F;

    delete colorBuffer;      colorBuffer = nullptr;
    delete depthBuffer;      depthBuffer = nullptr;
    delete forwardFBO;       forwardFBO = nullptr;

    TextureType texType = (MultiSampleCount > 0)
        ? TextureType::Texture2DMultisample
        : TextureType::Texture2D;

    constexpr uint64_t kSamplerFlags =
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
        | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;

    colorBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        colorFmt, texType,
        /*sampleDepth=*/false,
        kSamplerFlags,
        MultiSampleCount > 0 ? MultiSampleCount : 0);
    colorBuffer->SetName("MainColorBuffer");

    depthBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        TextureFormat::Depth24, texType,
        /*sampleDepth=*/false,
        kSamplerFlags,
        MultiSampleCount > 0 ? MultiSampleCount : 0);
    depthBuffer->SetName("MainDepthBuffer");

    // FIX: Depth pre-pass gets its own FBO with its own view ID.
    // This is a depth-only FBO — no color attachment.
    // bgfx will execute this view before forwardFBO's view because its
    // ID is allocated first (lower number).

    // Color pass FBO — color + the same depth buffer written by the pre-pass.
    forwardFBO = new Framebuffer();
    forwardFBO->attachColor(colorBuffer, 0u);
    forwardFBO->attachDepth(depthBuffer);
}

// -----------------------------------------------------------------------
// InitResolveFrameBuffers
// -----------------------------------------------------------------------
void Renderer::InitResolveFrameBuffers()
{
    ivec2 screenResolution = GetScreenResolution();
    TextureFormat colorFmt = TextureFormat::RGBA16F;

    constexpr uint64_t kSamplerFlags =
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
        | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;

    colorResolveBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        colorFmt, TextureType::Texture2D,
        false, kSamplerFlags);
    colorResolveBuffer->SetName("ColorResolveBuffer");

    customIdResolveBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        TextureFormat::RGBA8, TextureType::Texture2D,
        false, kSamplerFlags);
    customIdResolveBuffer->SetName("CustomIdResolveBuffer");

    depthResolveBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        TextureFormat::Depth24, TextureType::Texture2D,
        false, kSamplerFlags);
    depthResolveBuffer->SetName("DepthResolveBuffer");

    forwardResolveFBO = new Framebuffer();
    forwardResolveFBO->attachColor(colorResolveBuffer, 0u);
    forwardResolveFBO->attachDepth(depthResolveBuffer);

    customIdFBO = new Framebuffer();
    customIdFBO->attachColor(customIdResolveBuffer, 0u);
}