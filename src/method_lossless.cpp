#include "method.hpp"
#include "util.hpp"
#include "wrapper.hpp"
#include <cassert>
#include <cstdint>
#include <span>

template <class LosslessWrapper> size_t Lossless<LosslessWrapper>::compress(const std::vector<float> &input)
{
    compressed_buffer = LosslessWrapper::compress(std::as_bytes(std::span(input)));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

template <class LosslessWrapper> std::span<const float> Lossless<LosslessWrapper>::decompress()
{
    decompressed_buffer = LosslessWrapper::decompress(compressed_buffer);
    return as_float_span(decompressed_buffer);
}

template class Lossless<Bsc>;
template class Lossless<Zstd>;
template class Lossless<Lz4>;
