#include "method.hpp"
#include "util.hpp"
#include "wrapper.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

size_t Zstd::compress(const std::vector<float> &input)
{
    compressed_buffer = ZstdWrapper::compress(std::as_bytes(std::span(input)));
    return compressed_buffer.size();
}

std::span<const float> Zstd::decompress()
{
    decompressed_buffer = ZstdWrapper::decompress(compressed_buffer);
    return as_float_span(decompressed_buffer);
}
