#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>


namespace ByteCompressor
{
    // Compress a byte array using zlib
    std::vector<uint8_t> CompressData(const std::vector<uint8_t>& input);

    // Decompress a byte array using zlib
    std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& input);
    
}
