#include "method.hpp"
#include "util.hpp"
#include <cstddef>
#include <cstdint>
#include <ieee754.h>
#include <immintrin.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <system_error>
#include <vector>
#include <chrono>
#include <ostream>

void quantize(const float* input, size_t sz, float* out, float e) {
    ieee754_float error{e};
    auto error_exp = error.ieee.exponent;
    size_t i = 0;

    for (; i < sz; i++)
    {
        uint32_t mant_mask = 0x007FFFFF;
        ieee754_float v = {input[i]};
        if (v.ieee.exponent > error_exp)
            mant_mask = mant_mask >> (v.ieee.exponent - error_exp);
        v.ieee.mantissa &= ~(mant_mask);
        out[i] = v.f;
    }
}

template <> size_t Mask<float>::compress(std::span<const float> input)
{
    std::unique_ptr<float[]> out(new float[input.size()]);
    // TOOD FIX
    // uint32_t mant_mask = 0x007FFFFF;
    // size_t i = 0;

    // __m256i mant_mask_v = _mm256_set1_epi32(mant_mask);
    // __m256i error_exp_v = _mm256_set1_epi32(error_exp);
    // __m256i exp_mask_v = _mm256_set1_epi32(0xFF);
    // for (; i + 8 <= input.size(); i += 8)
    // {
    //     __m256i v = _mm256_loadu_si256((__m256i *)&input[i]);
    //     __m256i v_exp = _mm256_and_si256(_mm256_srli_epi32(v, 23), exp_mask_v);
    //     __m256i exp_diff = _mm256_sub_epi32(v_exp, error_exp_v);
    //     __m256i v_mask_noshift = _mm256_andnot_si256(mant_mask_v, v);
    //     __m256i v_mask_shifted = _mm256_andnot_si256(_mm256_srlv_epi32(mant_mask_v, exp_diff), v);
    //     __m256i result = _mm256_blendv_epi8(v_mask_noshift, v_mask_shifted, _mm256_cmpgt_epi32(v_exp, error_exp_v));
    //     _mm256_storeu_si256((__m256i *)&out.get()[i], result);
    // }
    // for (; i < input.size(); i++)
    // {
    //     ieee754_float v = {input[i]};
    //     if (v.ieee.exponent > error_exp)
    //         v.ieee.mantissa &= ~(mant_mask >> (v.ieee.exponent - error_exp));
    //     else
    //         v.ieee.mantissa &= ~(mant_mask);
    //     out.get()[i] = v.f;
    // }

    quantize(&input[0], input.size(), &out[0], Method::error);

    compressed_buffer = encoding->encode(std::as_bytes(std::span(out.get(), input.size())));
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
