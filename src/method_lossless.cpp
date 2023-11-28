#include "method.hpp"
#include "util.hpp"
#include "encoding.hpp"
#include <cassert>
#include <cstdint>
#include <span>

size_t Lossless::compress(const std::vector<float> &input)
{
    compressed_buffer = encoding.encode(std::as_bytes(std::span(input)));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

std::span<const float> Lossless::decompress()
{
    decompressed_buffer = encoding.decode(compressed_buffer);
    return as_float_span(decompressed_buffer);
}
