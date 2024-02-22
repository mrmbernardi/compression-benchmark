#include "encoding.hpp"
#include "method.hpp"
#include "util.hpp"
#include <cassert>
#include <cstdint>
#include <span>

template <typename F> size_t Lossless<F>::compress(std::span<const F> input)
{
    compressed_span = encoding->encode(std::as_bytes(input));
    return compressed_span.size_bytes();
}

template <typename F> std::span<const F> Lossless<F>::decompress()
{
    return as_typed_span<F>(encoding->decode(compressed_span));
}
template class Lossless<float>;
template class Lossless<double>;