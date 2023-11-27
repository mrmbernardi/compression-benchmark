#include "lz4.h"
#include "wrapper.hpp"
#include <cassert>
#include <cstddef>
#include <stdexcept>

std::vector<std::byte> Lz4::encode(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    std::vector<std::byte> output_buffer(LZ4_compressBound(input_sz) + sizeof(size_t));
    auto input_ptr = reinterpret_cast<const char *>(input.data());
    auto output_ptr = reinterpret_cast<char *>(output_buffer.data() + sizeof(size_t));
    size_t compressed_sz = LZ4_compress_default(input_ptr, output_ptr, input_sz, output_buffer.size() - sizeof(size_t));
    if (compressed_sz == 0)
    {
        throw std::runtime_error("lz4 compression failed");
    }
    auto sz_span = std::as_bytes(std::span<size_t, 1>(&input_sz, 1));
    std::copy(sz_span.begin(), sz_span.end(), output_buffer.begin());
    output_buffer.resize(compressed_sz + sizeof(size_t));
    return output_buffer;
}

std::vector<std::byte> Lz4::decode(std::span<const std::byte> input)
{
    size_t decompressed_sz = *reinterpret_cast<const size_t *>(input.data());
    std::vector<std::byte> output_buffer(decompressed_sz);
    auto input_ptr = reinterpret_cast<const char *>(input.data() + sizeof(size_t));
    auto output_ptr = reinterpret_cast<char *>(output_buffer.data());
    size_t res = LZ4_decompress_safe(input_ptr, output_ptr, input.size() - sizeof(size_t), output_buffer.size());
    if (res < 0)
    {
        throw std::runtime_error("lz4 decompression failed");
    }
    return output_buffer;
}