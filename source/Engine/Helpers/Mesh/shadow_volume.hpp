#pragma once

// shadow_volume.hpp — see shadow_volume.cpp for full design notes.

#include <glm.h>
#include <bgfx/bgfx.h>
#include <vector>

namespace roj { struct SkinnedMesh; }

namespace roj
{

struct ShadowVolumePrecomp
{
    // Cap geometry — indexed triangle list, VertexData layout.
    bgfx::VertexBufferHandle capVbh        = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  capIbh        = BGFX_INVALID_HANDLE;
    uint32_t                 capIndexCount = 0;

    // Edge geometry — indexed triangle list, VertexData layout.
    bgfx::VertexBufferHandle edgeVbh        = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  edgeIbh        = BGFX_INVALID_HANDLE;
    uint32_t                 edgeIndexCount = 0;

    ShadowVolumePrecomp() = default;
    ShadowVolumePrecomp(const ShadowVolumePrecomp&)            = delete;
    ShadowVolumePrecomp& operator=(const ShadowVolumePrecomp&) = delete;

    ShadowVolumePrecomp(ShadowVolumePrecomp&& o) noexcept
        : capVbh(o.capVbh),   capIbh(o.capIbh),   capIndexCount(o.capIndexCount),
          edgeVbh(o.edgeVbh), edgeIbh(o.edgeIbh), edgeIndexCount(o.edgeIndexCount)
    {
        o.capVbh = o.edgeVbh = BGFX_INVALID_HANDLE;
        o.capIbh = o.edgeIbh = BGFX_INVALID_HANDLE;
        o.capIndexCount = o.edgeIndexCount = 0;
    }

    ShadowVolumePrecomp& operator=(ShadowVolumePrecomp&& o) noexcept
    {
        if (this != &o) { this->~ShadowVolumePrecomp(); new(this) ShadowVolumePrecomp(std::move(o)); }
        return *this;
    }

    ~ShadowVolumePrecomp()
    {
        if (bgfx::isValid(capVbh))  bgfx::destroy(capVbh);
        if (bgfx::isValid(capIbh))  bgfx::destroy(capIbh);
        if (bgfx::isValid(edgeVbh)) bgfx::destroy(edgeVbh);
        if (bgfx::isValid(edgeIbh)) bgfx::destroy(edgeIbh);
    }
};

// Build once at load time while mesh.vertices/indices are still populated.
ShadowVolumePrecomp BuildShadowVolumePrecomp(const SkinnedMesh& mesh);

} // namespace roj
