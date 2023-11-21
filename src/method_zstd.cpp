#include "method.hpp"
#include "zstd.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

size_t Zstd::compress(const std::vector<float> &input)
{
    size_t input_sz = input.size() * sizeof(input[0]);
    compressed_buffer = std::vector<std::byte>(ZSTD_compressBound(input_sz) + sizeof(size_t));
    size_t compressed_sz =
        ZSTD_compress(compressed_buffer.data() + sizeof(size_t), compressed_buffer.size(), input.data(), input_sz, 3);
    assert(!ZSTD_isError(compressed_sz));
    auto sz_span = std::as_bytes(std::span<size_t, 1>(&input_sz, 1));
    std::copy(sz_span.begin(), sz_span.end(), compressed_buffer.begin());
    compressed_buffer.resize(compressed_sz + sizeof(size_t));
    return compressed_sz;
}

std::span<const float> Zstd::decompress()
{
    size_t decompressed_sz = *reinterpret_cast<const size_t *>(compressed_buffer.data());
    assert(decompressed_sz % sizeof(float) == 0);
    decompressed_buffer = std::vector<float>(decompressed_sz / sizeof(float));
    size_t res = ZSTD_decompress(decompressed_buffer.data(), decompressed_sz, compressed_buffer.data() + sizeof(size_t),
                                 compressed_buffer.size() - sizeof(size_t));
    assert(!ZSTD_isError(res));
    return std::span<const float>(decompressed_buffer);
}
