#pragma once
#include "NativeFileSystem.h"
#include <optional>

class EmscriptenFileSystem : public NativeFileSystem {
public:
    EmscriptenFileSystem();
    ~EmscriptenFileSystem() override = default;

    bool Init() override;

    // Text overrides
    bool WriteSaveFile(const std::string& path, const std::string& content) override;
    std::optional<std::string> ReadSaveFile(const std::string& path) override;

    // Binary overrides (routing through text)
    bool WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) override;
    std::optional<std::vector<uint8_t>> ReadSaveFileBinary(const std::string& path) override;
};