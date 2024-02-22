#include "method.hpp"
#include "util.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ieee754.h>
#include <immintrin.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <system_error>
#include <vector>

void quantize(const float *input, size_t sz, float *out, float e)
{
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

void quantize(const double *input, size_t sz, double *out, double e)
{
    // TODO
}

template <typename F> size_t Mask<F>::compress(std::span<const F> input)
{
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

    std::unique_ptr<F[]> out(new F[input.size()]);
    quantize(&input[0], input.size(), &out[0], Method<F>::error);
    compressed_span = encoding->encode(std::as_bytes(std::span(out.get(), input.size())));
    return compressed_span.size_bytes();
}

template <typename F> std::span<const F> Mask<F>::decompress()
{
    return as_typed_span<F>(encoding->decode(compressed_span));
}
template class Mask<float>;
template class Mask<double>;
