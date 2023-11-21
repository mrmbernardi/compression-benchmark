#include "bsc_wrapper.hpp"
#include "method.hpp"
#include <cassert>
#include <cstdint>
#include <span>

size_t Bsc::compress(const std::vector<float> &input)
{
    compressed_buffer = bsc_compress_wrapper(std::as_bytes(std::span(input)));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

std::span<const float> Bsc::decompress()
{
    decompressed_buffer = bsc_decompress_wrapper(compressed_buffer);
    assert(decompressed_buffer.size() % sizeof(float) == 0);
    auto data_ptr = reinterpret_cast<float *>(decompressed_buffer.data());
    return std::span<const float>(data_ptr, decompressed_buffer.size() / sizeof(float));
}
