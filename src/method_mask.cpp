#include "method.hpp"
#include "util.hpp"
#include <cstdint>
#include <ieee754.h>
#include <system_error>
#include <vector>

template <> size_t Mask<float>::compress(std::span<const float> input)
{
    ieee754_float error{Method::error};
    std::vector<float> out;
    out.reserve(input.size());
    for (float f : input)
    {
        ieee754_float v = {f};
        uint32_t mant_mask = 0x007FFFFF;
        if (v.ieee.exponent > error.ieee.exponent)
            mant_mask = mant_mask >> (v.ieee.exponent - error.ieee.exponent);
        v.ieee.mantissa &= ~(mant_mask);
        out.push_back(v.f);
    }

    compressed_buffer = encoding->encode(std::as_bytes(std::span(out)));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

template <> size_t Mask<double>::compress(std::span<const double> input)
{
    // TODO: masking code

    compressed_buffer = encoding->encode(std::as_bytes(input));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

template <typename F> std::span<const F> Mask<F>::decompress()
{
    decompressed_buffer = encoding->decode(compressed_buffer);
    return as_span<F>(decompressed_buffer);
}
template class Mask<float>;
template class Mask<double>;
