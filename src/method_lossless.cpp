#include "encoding.hpp"
#include "method.hpp"
#include "util.hpp"
#include <cassert>
#include <cstdint>
#include <span>

template <typename F> size_t Lossless<F>::compress(std::span<const F> input)
{
    compressed_buffer = encoding->encode(std::as_bytes(input));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

template <typename F> std::span<const F> Lossless<F>::decompress()
{
    decompressed_buffer = encoding->decode(compressed_buffer);
    return as_span<F>(decompressed_buffer);
}
template class Lossless<float>;
template class Lossless<double>;