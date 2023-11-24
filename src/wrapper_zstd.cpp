#include "wrapper.hpp"
#include "zstd.h"
#include <cassert>
#include <cstddef>
#include <stdexcept>

std::vector<std::byte> Zstd::compress(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    std::vector<std::byte> output_buffer(ZSTD_compressBound(input_sz) + sizeof(size_t));
    size_t compressed_sz = ZSTD_compress(output_buffer.data() + sizeof(size_t), output_buffer.size() - sizeof(size_t),
                                         input.data(), input_sz, 3);
    if (ZSTD_isError(compressed_sz))
    {
        throw std::runtime_error("ZSTD compression failed");
    }
    auto sz_span = std::as_bytes(std::span<size_t, 1>(&input_sz, 1));
    std::copy(sz_span.begin(), sz_span.end(), output_buffer.begin());
    output_buffer.resize(compressed_sz + sizeof(size_t));
    return output_buffer;
}

std::vector<std::byte> Zstd::decompress(std::span<const std::byte> input)
{
    size_t decompressed_sz = *reinterpret_cast<const size_t *>(input.data());
    std::vector<std::byte> output_buffer(decompressed_sz);
    size_t res = ZSTD_decompress(output_buffer.data(), decompressed_sz, input.data() + sizeof(size_t),
                                 input.size() - sizeof(size_t));
    if (ZSTD_isError(res))
    {
        throw std::runtime_error("ZSTD decompression failed");
    }
    return output_buffer;
}