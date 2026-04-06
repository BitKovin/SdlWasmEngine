#include "UiBilboard.h"

#include "../UiRenderer.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

// ---------------------------------------------------------------------------
// Atlas implementation
// ---------------------------------------------------------------------------
UiBilboardAtlas::UiBilboardAtlas(ivec2 size)
{
    texture = new RenderTexture(
        size.x,
        size.y,
        TextureFormat::RGBA8,
        TextureType::Texture2D,
        false,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
        BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT
    );

    texture->SetName("UiBillboardAtlas " + to_string(size.x) + "x" + to_string(size.y));

    // initial free space = whole atlas
    freeRects.push_back({ 0, 0, size.x, size.y });
}

UiBilboardAtlas::~UiBilboardAtlas()
{
    delete texture;
}

bool UiBilboardAtlas::TryAllocate(ivec2 reqSize, BillboardAllocation& outAlloc)
{
    for (size_t i = 0; i < freeRects.size(); ++i)
    {
        FreeRect& fr = freeRects[i];
        if (reqSize.x <= fr.w && reqSize.y <= fr.h)
        {
            int x = fr.x;
            int y = fr.y;
            int w = reqSize.x;
            int h = reqSize.y;

            // split remaining space (classic guillotine split)
            if (fr.w > w)
                freeRects.push_back({ x + w, y, fr.w - w, fr.h });      // right strip
            if (fr.h > h)
                freeRects.push_back({ x, y + h, w, fr.h - h });         // bottom strip

            // remove used free rect
            freeRects[i] = freeRects.back();
            freeRects.pop_back();

            // fill allocation
            outAlloc.atlas = this;
            outAlloc.rect = ivec4(x, y, w, h);

            float tw = (float)texture->width();
            float th = (float)texture->height();
            outAlloc.uvRect = vec4(
                (float)x / tw,          // minU
                (float)y / th,          // minV
                (float)(x + w) / tw,    // maxU
                (float)(y + h) / th     // maxV
            );

            return true;
        }
    }
    return false;
}

void UiBilboardAtlas::Free(const BillboardAllocation& alloc)
{
    if (alloc.atlas != this || !alloc.IsValid()) return;

    FreeRect fr{ alloc.rect.x, alloc.rect.y, alloc.rect.z, alloc.rect.w };
    freeRects.push_back(fr);
    // (no merging for simplicity – fragmentation is rare with UI billboards)
}

// ---------------------------------------------------------------------------
UiBillboardAtlasManager& UiBillboardAtlasManager::Get()
{
    static UiBillboardAtlasManager instance;
    return instance;
}

BillboardAllocation UiBillboardAtlasManager::Allocate(ivec2 size)
{
    // try existing atlases
    for (auto& atlas : atlases)
    {
        BillboardAllocation alloc;
        if (atlas->TryAllocate(size, alloc))
            return alloc;
    }

    // no room → create a new atlas
    auto newAtlas = std::make_unique<UiBilboardAtlas>(kAtlasSize);
    atlases.push_back(std::move(newAtlas));

    BillboardAllocation alloc;
    if (atlases.back()->TryAllocate(size, alloc))
        return alloc;

    // should never happen (billboard size is guaranteed ≤ atlas size)
    return {};
}

void UiBillboardAtlasManager::Free(BillboardAllocation& alloc)
{
    if (!alloc.IsValid()) return;
    alloc.atlas->Free(alloc);
    alloc.atlas = nullptr; // invalidate
}

// ---------------------------------------------------------------------------
// UiBilboard implementation (now atlas-based)
// ---------------------------------------------------------------------------
UiBilboard::~UiBilboard()
{
    UiBillboardAtlasManager::Get().Free(allocation);
}

void UiBilboard::DrawForward(mat4x4 view, mat4x4 projection)
{
    if (!allocation.IsValid())
        return;

    // Point StaticMesh to the shared atlas texture
    ColorTextureId = (int)allocation.atlas->GetTexture()->textureHandle().idx;

    // -------------------------------------------------------------------
    // Set the atlas rectangle uniform (one-time creation)
    // -------------------------------------------------------------------
    static bgfx::UniformHandle uAtlasRect = BGFX_INVALID_HANDLE;
    if (uAtlasRect.idx == bgfx::kInvalidHandle)
    {
        uAtlasRect = bgfx::createUniform("u_atlasRect", bgfx::UniformType::Vec4);
    }

    // Send the pre-computed minU/minV/maxU/maxV for this billboard
    bgfx::setUniform(uAtlasRect, &allocation.uvRect);

    // Let the normal StaticMesh pipeline do the rest (it already knows how to
    // bind the texture we just set and will use the new fs_unlit_rect shader)
    StaticMesh::DrawForward(view, projection);
}

void UiBilboard::FinalizeFrameData()
{
    Rotation = Camera::rotation;

    Canvas.size = vec2((float)ViewportSize.x, (float)ViewportSize.y);

    Scale.x = (float)ViewportSize.x / PixelPerMeter;
    Scale.y = (float)ViewportSize.y / PixelPerMeter;

    Canvas.FinalizeChildren();

    StaticMesh::FinalizeFrameData();
}

void UiBilboard::Update()
{
    Canvas.Update();
    Canvas.UpdateChildrenOffsetRecursive();
}

void UiBilboard::PreDraw()
{
    StaticMesh::PreDraw();

    DrawUi();
}

void UiBilboard::DrawUi()
{
    EnsureAtlasAllocation();
    if (!allocation.IsValid()) return;

    UiRenderer::customViewport = true;
    UiRenderer::customViewportSize = ViewportSize;

    // --- bind the shared atlas render target ---
    allocation.atlas->GetTexture()->setAsRenderTarget();

    // render only into this billboard’s sub-rectangle inside the atlas
    bgfx::setViewRect(
        ViewIdManager::GetCurrentId(),
        (uint16_t)allocation.rect.x,
        (uint16_t)allocation.rect.y,
        (uint16_t)ViewportSize.x,
        (uint16_t)ViewportSize.y
    );

    // Clear the sub-region to transparent black (same as original main UI pass)
    bgfx::setViewClear(
        ViewIdManager::GetCurrentId(),
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x00000000,
        1.0f, 0
    );

    auto savedState = BgfxStateManager::GetState();

    BgfxStateManager::Reset();
    BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
    BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);

    Canvas.Draw();
    UiElement::DrawingLate = true;
    for (auto elem : UiElement::pendingLateDrawElements)
    {
        elem->Draw();
    }
    UiElement::DrawingLate = false;
    UiElement::pendingLateDrawElements.clear();

    UiRenderer::EndFrame();

    BgfxStateManager::SetState(savedState);

    UiRenderer::customViewport = false;
}

void UiBilboard::EnsureAtlasAllocation()
{
    // already have a valid allocation of the correct size → nothing to do
    if (allocation.IsValid() &&
        allocation.rect.z == ViewportSize.x &&
        allocation.rect.w == ViewportSize.y)
        return;

    // size changed (or first time) → free old slot and get a new one
    if (allocation.IsValid())
        UiBillboardAtlasManager::Get().Free(allocation);

    allocation = UiBillboardAtlasManager::Get().Allocate(ViewportSize);
}