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
    bool unregisterResource(ResourceType type, uint64_t id) {
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

    // Type -> (ID -> ResourceEntry)
    std::unordered_map<ResourceType, std::unordered_map<uint64_t, ResourceEntry>> resources_;
    std::unordered_map<ResourceType, size_t> totalMemoryByType_;
    size_t totalMemory_ = 0;
};