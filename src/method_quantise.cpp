#include "method.hpp"
#include "wrapper.hpp"
#include "util.hpp"

template <class LosslessWrapper> size_t Quantise<LosslessWrapper>::compress(const std::vector<float> &input)
{
    // TOOD
    compressed_buffer = LosslessWrapper::compress(std::as_bytes(std::span(input)));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

template <class LosslessWrapper> std::span<const float> Quantise<LosslessWrapper>::decompress()
{
    // TOOD
    decompressed_buffer = LosslessWrapper::decompress(compressed_buffer);
    return as_float_span(decompressed_buffer);
}

template class Quantise<Bsc>;
template class Quantise<Zstd>;