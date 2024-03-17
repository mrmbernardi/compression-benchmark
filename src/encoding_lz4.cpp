#include "lz4.h"
#include "encoding.hpp"
#include <cstddef>
#include <stdexcept>

std::span<const std::byte> Lz4::encode(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    compressed_buffer = std::vector<std::byte>(LZ4_compressBound(input_sz) + sizeof(size_t));
    auto input_ptr = reinterpret_cast<const char *>(input.data());
    auto output_ptr = reinterpret_cast<char *>(compressed_buffer.data() + sizeof(size_t));
    size_t compressed_sz = LZ4_compress_default(input_ptr, output_ptr, input_sz, compressed_buffer.size() - sizeof(size_t));
    if (compressed_sz == 0)
    {
        throw std::runtime_error("lz4 compression failed");
    }
    auto sz_span = std::as_bytes(std::span<size_t, 1>(&input_sz, 1));
    std::copy(sz_span.begin(), sz_span.end(), compressed_buffer.begin());
    compressed_buffer.resize(compressed_sz + sizeof(size_t));
    return compressed_buffer;
}

std::span<const std::byte> Lz4::decode(std::span<const std::byte> input)
{
    size_t decompressed_sz = *reinterpret_cast<const size_t *>(input.data());
    decompressed_buffer = std::vector<std::byte>(decompressed_sz);
    auto input_ptr = reinterpret_cast<const char *>(input.data() + sizeof(size_t));
    auto output_ptr = reinterpret_cast<char *>(decompressed_buffer.data());
    size_t res = LZ4_decompress_safe(input_ptr, output_ptr, input.size() - sizeof(size_t), decompressed_buffer.size());
    if (res < 0)
    {
        throw std::runtime_error("lz4 decompression failed");
    }
    return decompressed_buffer;
}