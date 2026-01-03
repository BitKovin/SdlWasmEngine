#include <RmlUi/Core.h>
#include <vector>
#include <string>
#include <algorithm>

#include "../../../FileSystem/FileSystem.h"

class RmlFileInterface : public Rml::FileInterface
{
public:
    struct FileData
    {
        std::vector<uint8_t> data;
        size_t offset = 0; // current read position
    };

    // Opens a file and returns a handle
    Rml::FileHandle Open(const Rml::String& path) override
    {
        std::vector<uint8_t> file_bytes = FileSystemEngine::ReadFileBinary(path);
        if (file_bytes.empty())
            return 0; // 0 = invalid handle

        auto file_data = new FileData{ std::move(file_bytes), 0 };
        return reinterpret_cast<Rml::FileHandle>(file_data);
    }

    // Closes a previously opened file
    void Close(Rml::FileHandle file) override
    {
        if (!file) return;
        auto file_data = reinterpret_cast<FileData*>(file);
        delete file_data;
    }

    // Reads data from an opened file
    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override
    {
        if (!file || size == 0) return 0;
        auto file_data = reinterpret_cast<FileData*>(file);

        size_t remaining = file_data->data.size() - file_data->offset;
        size_t to_read = std::min(size, remaining);
        if (to_read == 0) return 0;

        memcpy(buffer, file_data->data.data() + file_data->offset, to_read);
        file_data->offset += to_read;
        return to_read;
    }

    // Seeks within the file
    bool Seek(Rml::FileHandle file, long offset, int origin) override
    {
        if (!file) return false;
        auto file_data = reinterpret_cast<FileData*>(file);
        size_t new_offset = 0;

        switch (origin)
        {
        case SEEK_SET: new_offset = offset; break;
        case SEEK_CUR: new_offset = file_data->offset + offset; break;
        case SEEK_END: new_offset = file_data->data.size() + offset; break;
        default: return false;
        }

        if (new_offset > file_data->data.size()) return false;
        file_data->offset = new_offset;
        return true;
    }

    // Returns current position
    size_t Tell(Rml::FileHandle file) override
    {
        if (!file) return 0;
        auto file_data = reinterpret_cast<FileData*>(file);
        return file_data->offset;
    }

    // Returns length of file
    size_t Length(Rml::FileHandle file) override
    {
        if (!file) return 0;
        auto file_data = reinterpret_cast<FileData*>(file);
        return file_data->data.size();
    }

    // Loads a file fully into a string
    bool LoadFile(const Rml::String& path, Rml::String& out_data) override
    {
        std::vector<uint8_t> file_bytes = FileSystemEngine::ReadFileBinary(path);
        if (file_bytes.empty())
            return false;

        out_data.assign(file_bytes.begin(), file_bytes.end());
        return true;
    }
};
