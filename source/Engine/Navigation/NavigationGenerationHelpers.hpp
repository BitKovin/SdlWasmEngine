#pragma once 

#include "Detour/DetourTileCacheBuilder.h"
#include <cstring>

// Custom compressor (simplified, no real compression)
struct FastLZCompressor : public dtTileCacheCompressor {
    int maxCompressedSize(const int size) override { return static_cast<int>(size * 1.05f); }
    dtStatus compress(const unsigned char* buffer, const int bufferSize,
        unsigned char* compressed, const int maxCompressedSize, int* compressedSize) override {
        if (bufferSize > maxCompressedSize) return DT_BUFFER_TOO_SMALL;
        memcpy(compressed, buffer, bufferSize);
        *compressedSize = bufferSize;
        return DT_SUCCESS;
    }
    dtStatus decompress(const unsigned char* compressed, const int compressedSize,
        unsigned char* buffer, const int maxBufferSize, int* bufferSize) override {
        if (compressedSize > maxBufferSize) return DT_BUFFER_TOO_SMALL;
        memcpy(buffer, compressed, compressedSize);
        *bufferSize = compressedSize;
        return DT_SUCCESS;
    }
};