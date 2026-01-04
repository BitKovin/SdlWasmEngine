#include "ByteCompressor.h"
#include "../Compression/miniz.h"

std::vector<uint8_t> ByteCompressor::CompressData(const std::vector<uint8_t>& input)
{
    if (input.empty())
        return {};

    uLongf compressedSize = compressBound(static_cast<uLong>(input.size()));
    std::vector<uint8_t> compressedData(compressedSize);

    int result = compress(
        compressedData.data(),
        &compressedSize,
        input.data(),
        static_cast<uLong>(input.size())
    );

    if (result != Z_OK)
        throw std::runtime_error("Compression failed");

    compressedData.resize(compressedSize); // Resize to actual compressed size
    return compressedData;
}

std::vector<uint8_t> ByteCompressor::DecompressData(const std::vector<uint8_t>& input)
{
    if (input.empty())
        return {};

    // Start with a buffer of 4x the compressed size (can grow if needed)
    uLongf decompressedSize = input.size() * 4;
    std::vector<uint8_t> decompressedData(decompressedSize);

    int result = Z_OK;
    while ((result = uncompress(
        decompressedData.data(),
        &decompressedSize,
        input.data(),
        static_cast<uLong>(input.size())
    )) == Z_BUF_ERROR)
    {
        // Buffer too small, increase size and retry
        decompressedSize *= 2;
        decompressedData.resize(decompressedSize);
    }

    if (result != Z_OK)
        throw std::runtime_error("Decompression failed");

    decompressedData.resize(decompressedSize); // Resize to actual decompressed size
    return decompressedData;
}
