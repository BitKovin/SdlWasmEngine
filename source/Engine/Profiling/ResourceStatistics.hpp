#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <optional>
#include <chrono>
#include <imgui/imgui.h>
#include <bgfx/bgfx.h>

#include <atomic>

enum class ResourceType {
    Texture,
    TextureCube,
    RenderTexture,
    VertexBuffer,
    IndexBuffer,
    InstanceBuffer
};

// Helper to convert enum to string for display
inline std::string resourceTypeToString(ResourceType type) {
    switch (type) {
    case ResourceType::Texture:         return "Texture";
    case ResourceType::TextureCube:     return "TextureCube";
    case ResourceType::RenderTexture:   return "RenderTexture";
    case ResourceType::VertexBuffer:    return "VertexBuffer";
    case ResourceType::IndexBuffer:     return "IndexBuffer";
    case ResourceType::InstanceBuffer:  return "InstanceBuffer";
    default:                            return "Unknown";
    }
}

class ResourceStatistics 
{
public:

    ~ResourceStatistics()
    {
        shutdown_ = true;
    }

    static ResourceStatistics& Instance() {
        static ResourceStatistics instance;
        return instance;
	}

    // Register a resource with type, ID, size, and optional name
    void registerResource(ResourceType type, uint64_t id, size_t sizeBytes, const std::string& name = "")
    {
        auto now = std::chrono::steady_clock::now();

        // Check if resource already exists
        auto typeIt = resources_.find(type);
        if (typeIt != resources_.end()) {
            auto resIt = typeIt->second.find(id);
            if (resIt != typeIt->second.end()) {
                // Resource exists - subtract old memory first
                size_t oldSize = resIt->second.sizeBytes;
                totalMemoryByType_[type] -= oldSize;
                totalMemory_ -= oldSize;
            }
        }

        // Register/update the resource
        ResourceEntry entry{ id, sizeBytes, name, type, now };
        resources_[type][id] = entry;
        totalMemoryByType_[type] += sizeBytes;
        totalMemory_ += sizeBytes;
    }

    // Unregister a resource
    bool unregisterResource(ResourceType type, uint64_t id) 
    {

		if (shutdown_) return false; // Prevent modifications during shutdown

        auto typeIt = resources_.find(type);
        if (typeIt == resources_.end()) return false;

        auto resIt = typeIt->second.find(id);
        if (resIt == typeIt->second.end()) return false;

        size_t size = resIt->second.sizeBytes;
        typeIt->second.erase(resIt);
        totalMemoryByType_[type] -= size;
        totalMemory_ -= size;

        // Clean up empty type entries
        if (typeIt->second.empty()) {
            resources_.erase(typeIt);
            totalMemoryByType_.erase(type);
        }

        return true;
    }

    // Update resource name
    bool setResourceName(ResourceType type, uint64_t id, const std::string& name) {
        auto typeIt = resources_.find(type);
        if (typeIt == resources_.end()) return false;

        auto resIt = typeIt->second.find(id);
        if (resIt == typeIt->second.end()) return false;

        resIt->second.name = name;
        return true;
    }

    // Get resource name
    std::optional<std::string> getResourceName(ResourceType type, uint64_t id) const {
        auto typeIt = resources_.find(type);
        if (typeIt == resources_.end()) return std::nullopt;

        auto resIt = typeIt->second.find(id);
        if (resIt == typeIt->second.end()) return std::nullopt;

        return resIt->second.name.empty() ? std::nullopt : std::make_optional(resIt->second.name);
    }

    // Get total memory usage
    size_t getTotalMemory() const {
        return totalMemory_;
    }

    // Real bgfx resource handles are uint16_t (0..65535), so any ID at or
    // above this boundary is guaranteed to be a synthetic/approximated entry
    // that doesn't correspond to an actual bgfx handle -- e.g. the backbuffer
    // in BgfxResetManager.h, which bgfx never exposes a handle for and which
    // therefore can never appear in bgfx's own internal counters (getStats()'s
    // textureMemoryUsed/rtMemoryUsed). Anything comparing our totals against
    // those bgfx counters needs to exclude this range or the comparison is
    // apples-to-oranges.
    static constexpr uint64_t kSyntheticIdBase = 0x10000;

    // Sum of only the entries backed by a real bgfx handle (id < kSyntheticIdBase).
    // Use this instead of getMemoryByType() when cross-checking against bgfx::getStats().
    size_t getRealMemoryByType(ResourceType type) const {
        auto it = resources_.find(type);
        if (it == resources_.end()) return 0;
        size_t total = 0;
        for (const auto& [id, entry] : it->second) {
            if (id < kSyntheticIdBase) total += entry.sizeBytes;
        }
        return total;
    }

    // Sum of only the synthetic/approximated entries (id >= kSyntheticIdBase),
    // e.g. the backbuffer. These are real VRAM that shows up in driver-reported
    // totals, but can never show up in bgfx's own per-handle counters.
    size_t getSyntheticMemoryByType(ResourceType type) const {
        auto it = resources_.find(type);
        if (it == resources_.end()) return 0;
        size_t total = 0;
        for (const auto& [id, entry] : it->second) {
            if (id >= kSyntheticIdBase) total += entry.sizeBytes;
        }
        return total;
    }

    // Get memory usage for a specific type
    size_t getMemoryByType(ResourceType type) const {
        auto it = totalMemoryByType_.find(type);
        return (it != totalMemoryByType_.end()) ? it->second : 0;
    }

    // Get count of resources by type
    size_t getResourceCount(ResourceType type) const {
        auto it = resources_.find(type);
        return (it != resources_.end()) ? it->second.size() : 0;
    }

    // Get total resource count across all types
    size_t getTotalResourceCount() const {
        size_t count = 0;
        for (const auto& [type, resMap] : resources_) {
            count += resMap.size();
        }
        return count;
    }

    // ImGui visualization
    void renderImGui(bool* p_open = nullptr) {
        if (!ImGui::Begin("Resource Statistics", p_open)) {
            ImGui::End();
            return;
        }

        // Summary header
        ImGui::Text("Total Memory: %.2f MB", totalMemory_ / (1024.0f * 1024.0f));
        ImGui::SameLine();
        ImGui::Text(" | Total Resources: %zu", getTotalResourceCount());

        // Cross-check against bgfx's own counters. Our number above is a
        // manual sum of every resource we explicitly register/unregister,
        // so it can only ever be as complete as our instrumentation. bgfx
        // itself tracks its own texture/render-target memory internally
        // (independent of our bookkeeping) and, on backends that support
        // it, the graphics driver reports the real committed VRAM for the
        // process. Neither of these is "wrong" if it disagrees with us --
        // they're the ground truth we're trying to match.
        {
            const bgfx::Stats* stats = bgfx::getStats();
            if (stats)
            {
                const float bgfxTexRtMB =
                    (stats->textureMemoryUsed > 0 ? stats->textureMemoryUsed : 0) / (1024.0f * 1024.0f) +
                    (stats->rtMemoryUsed     > 0 ? stats->rtMemoryUsed     : 0) / (1024.0f * 1024.0f);

                // Apples-to-apples: bgfx's counters only cover textures and
                // render targets IT created (i.e. resources with a real bgfx
                // handle), so compare against that same subset of our own
                // tracking -- not getMemoryByType(), which would also pull in
                // synthetic/approximated entries like the backbuffer that bgfx
                // structurally can never count (it never handed us a handle
                // for the swap chain in the first place).
                const float ourTexRtMB =
                    getRealMemoryByType(ResourceType::Texture)       / (1024.0f * 1024.0f) +
                    getRealMemoryByType(ResourceType::TextureCube)   / (1024.0f * 1024.0f) +
                    getRealMemoryByType(ResourceType::RenderTexture) / (1024.0f * 1024.0f);

                const float syntheticMB =
                    getSyntheticMemoryByType(ResourceType::Texture)       / (1024.0f * 1024.0f) +
                    getSyntheticMemoryByType(ResourceType::TextureCube)   / (1024.0f * 1024.0f) +
                    getSyntheticMemoryByType(ResourceType::RenderTexture) / (1024.0f * 1024.0f);

                ImGui::Text("bgfx-reported (textures + RTs): %.2f MB", bgfxTexRtMB);
                ImGui::SameLine();
                ImGui::Text(" | Ours, real bgfx handles only: %.2f MB", ourTexRtMB);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.6f, 0.85f, 1.0f, 1.0f), " (delta %.2f MB)", ourTexRtMB - bgfxTexRtMB);
                ImGui::TextDisabled("A non-zero delta here means we're missing (or double-counting) a real texture/RT creation site -- this is on us, not driver overhead.");
                if (syntheticMB > 0.0f)
                {
                    ImGui::Text("Approximated, no real bgfx handle (e.g. backbuffer): %.2f MB", syntheticMB);
                    ImGui::SameLine();
                    ImGui::TextDisabled("(excluded above by design -- bgfx can't count what it never gave us a handle for)");
                }

                if (stats->gpuMemoryUsed >= 0)
                {
                    ImGui::Text("Driver-reported GPU memory: %.2f MB", stats->gpuMemoryUsed / (1024.0f * 1024.0f));
                    if (stats->gpuMemoryMax > 0)
                    {
                        ImGui::SameLine();
                        ImGui::Text(" / %.2f MB max", stats->gpuMemoryMax / (1024.0f * 1024.0f));
                    }

                    // Whether the figure above is scoped to this process or
                    // reflects the whole GPU depends on which bgfx backend
                    // ended up active at runtime -- this engine doesn't force
                    // one, so we check and label it honestly rather than
                    // assuming.
                    const bgfx::RendererType::Enum renderer = bgfx::getRendererType();
                    switch (renderer)
                    {
                    case bgfx::RendererType::Direct3D11:
                    case bgfx::RendererType::Direct3D12:
                    case bgfx::RendererType::Metal:
                        ImGui::TextDisabled("(per-process on this backend -- comparable to Afterburner's per-process figure)");
                        break;
                    case bgfx::RendererType::OpenGL:
                    case bgfx::RendererType::OpenGLES:
                        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f),
                            "(WARNING: on OpenGL this is derived from total-minus-available VRAM -- "
                            "it reflects the whole GPU across every process, not just this one. "
                            "Not directly comparable to a per-process tool.)");
                        break;
                    case bgfx::RendererType::Vulkan:
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
                            "(nominally per-process on Vulkan, but driver behavior here has been "
                            "inconsistent -- verify against another tool before trusting it.)");
                        break;
                    default:
                        ImGui::TextDisabled("(per-process semantics unverified for this backend)");
                        break;
                    }

                    // Untracked = driver-reported minus bgfx's OWN internal
                    // texture/RT counters, not minus our manual total. bgfx's
                    // counters are the more trustworthy baseline here since
                    // they're independent of whether we've wired up every
                    // registration call site ourselves -- so this isolates
                    // memory that is structurally invisible to any texture/RT
                    // accounting at all: shader programs, uniform/constant
                    // buffers, vertex/index buffer bytes (bgfx doesn't expose
                    // a size for these either, only counts), the backbuffer if
                    // the backend doesn't fold it into rtMemoryUsed, command
                    // buffers, and driver/allocator overhead. If this is still
                    // large, that's the real, structural gap -- not a bug in
                    // our per-resource registration.
                    const float untracked = (stats->gpuMemoryUsed / (1024.0f * 1024.0f)) - bgfxTexRtMB;
                    ImGui::Text("Untracked beyond bgfx's texture/RT accounting: %.2f MB", untracked);
                    ImGui::TextDisabled("(shaders, uniforms, VB/IB bytes, backbuffer if not in rtMemoryUsed, driver overhead)");
                }
                else
                {
                    ImGui::TextDisabled("Driver-reported VRAM: not exposed by this bgfx backend");
                }
            }
        }

        ImGui::Separator();

        // Filter by type
        static int filterType = -1; // -1 means all types
        ImGui::Text("Filter by Type:");
        ImGui::SameLine();
        if (ImGui::RadioButton("All", filterType == -1)) filterType = -1;
        ImGui::SameLine();
        if (ImGui::RadioButton("Texture", filterType == 0)) filterType = 0;
        ImGui::SameLine();
        if (ImGui::RadioButton("TextureCube", filterType == 1)) filterType = 1;
        ImGui::SameLine();
        if (ImGui::RadioButton("RenderTexture", filterType == 2)) filterType = 2;

        if (ImGui::RadioButton("VertexBuffer", filterType == 3)) filterType = 3;
        ImGui::SameLine();
        if (ImGui::RadioButton("IndexBuffer", filterType == 4)) filterType = 4;
        ImGui::SameLine();
        if (ImGui::RadioButton("InstanceBuffer", filterType == 5)) filterType = 5;

        ImGui::Separator();

        // Collect all resources into a vector for sorting
        std::vector<const ResourceEntry*> allResources;
        for (const auto& [type, resMap] : resources_) {
            if (filterType == -1 || static_cast<ResourceType>(filterType) == type) {
                for (const auto& [id, entry] : resMap) {
                    allResources.push_back(&entry);
                }
            }
        }

        // Sorting controls
        static int sortColumn = 0; // 0=Type, 1=ID, 2=Name, 3=Size, 4=Creation Time
        static bool sortDescending = false;

        // Sort the resources
        std::sort(allResources.begin(), allResources.end(),
            [](const ResourceEntry* a, const ResourceEntry* b) {
                switch (sortColumn) {
                case 0: // Type
                    return sortDescending ? (a->type > b->type) : (a->type < b->type);
                case 1: // ID
                    return sortDescending ? (a->id > b->id) : (a->id < b->id);
                case 2: // Name
                    return sortDescending ? (a->name > b->name) : (a->name < b->name);
                case 3: // Size
                    return sortDescending ? (a->sizeBytes > b->sizeBytes) : (a->sizeBytes < b->sizeBytes);
                case 4: // Creation Time
                    return sortDescending ? (a->creationTime > b->creationTime) : (a->creationTime < b->creationTime);
                default:
                    return false;
                }
            });

        // Table
        if (ImGui::BeginTable("ResourceTable", 5,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable |
            ImGuiTableFlags_ScrollY)) {

            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Size (MB)", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Age (s)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row

            // Custom header with sorting
            ImGui::TableHeadersRow();

            // Check for header clicks to change sorting
            if (ImGui::TableGetSortSpecs() != nullptr) {
                ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
                if (sortSpecs->SpecsDirty) {
                    if (sortSpecs->SpecsCount > 0) {
                        sortColumn = sortSpecs->Specs[0].ColumnIndex;
                        sortDescending = sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Descending;
                    }
                    sortSpecs->SpecsDirty = false;
                }
            }

            // Render rows
            auto now = std::chrono::steady_clock::now();
            for (const ResourceEntry* entry : allResources) {
                ImGui::TableNextRow();

                // Type
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", resourceTypeToString(entry->type).c_str());

                // ID
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%llu", static_cast<unsigned long long>(entry->id));

                // Name
                ImGui::TableSetColumnIndex(2);
                if (entry->name.empty()) {
                    ImGui::TextDisabled("<unnamed>");
                }
                else {
                    ImGui::Text("%s", entry->name.c_str());
                }

                // Size in MB
                ImGui::TableSetColumnIndex(3);
                float sizeMB = entry->sizeBytes / (1024.0f * 1024.0f);
                ImGui::Text("%.3f", sizeMB);

                // Age in seconds
                ImGui::TableSetColumnIndex(4);
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - entry->creationTime);
                ImGui::Text("%lld", static_cast<long long>(duration.count()));
            }

            ImGui::EndTable();
        }

        ImGui::Separator();

        // Summary by type
        if (ImGui::CollapsingHeader("Summary by Type", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("SummaryTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Memory (MB)", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableHeadersRow();

                for (const auto& [type, memory] : totalMemoryByType_) {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", resourceTypeToString(type).c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%zu", getResourceCount(type));

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.3f", memory / (1024.0f * 1024.0f));
                }

                ImGui::EndTable();
            }
        }

        ImGui::End();
    }

    // Clear all statistics
    void clear() {
        resources_.clear();
        totalMemoryByType_.clear();
        totalMemory_ = 0;
    }

private:
    struct ResourceEntry {
        uint64_t id;
        size_t sizeBytes;
        std::string name;
        ResourceType type;
        std::chrono::steady_clock::time_point creationTime;
    };

    static inline std::atomic<bool> shutdown_;

    // Type -> (ID -> ResourceEntry)
    std::unordered_map<ResourceType, std::unordered_map<uint64_t, ResourceEntry>> resources_;
    std::unordered_map<ResourceType, size_t> totalMemoryByType_;
    size_t totalMemory_ = 0;
};