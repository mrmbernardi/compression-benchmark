#include "encoding.hpp"
#include "zstd.h"
#include <cassert>
#include <cstddef>
#include <stdexcept>

constexpr int level = 3;

std::string Zstd::name()
{
    return "Zstd (" + std::to_string(level) + ")";
}

std::span<const std::byte> Zstd::encode(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    compressed_buffer = std::vector<std::byte>(ZSTD_compressBound(input_sz) + sizeof(size_t));
    size_t compressed_sz = ZSTD_compress(compressed_buffer.data() + sizeof(size_t),
                                         compressed_buffer.size() - sizeof(size_t), input.data(), input_sz, level);
    if (ZSTD_isError(compressed_sz))
    {
        throw std::runtime_error("ZSTD compression failed");
    }
    auto sz_span = std::as_bytes(std::span<size_t, 1>(&input_sz, 1));
    std::copy(sz_span.begin(), sz_span.end(), compressed_buffer.begin());
    compressed_buffer.resize(compressed_sz + sizeof(size_t));
    return compressed_buffer;
}

std::span<const std::byte> Zstd::decode(std::span<const std::byte> input)
{
    size_t decompressed_sz = *reinterpret_cast<const size_t *>(input.data());
    decompressed_buffer = std::vector<std::byte>(decompressed_sz);
    size_t res = ZSTD_decompress(decompressed_buffer.data(), decompressed_sz, input.data() + sizeof(size_t),
                                 input.size() - sizeof(size_t));
    if (ZSTD_isError(res))
    {
        throw std::runtime_error("ZSTD decompression failed");
    }
    return decompressed_buffer;
}