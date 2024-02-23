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

void mask(const float *input, size_t sz, float *out, float e)
{
    const ieee754_float error{e};
    const auto error_exp = error.ieee.exponent;
    const uint32_t mant_mask = 0x007FFFFF;
    size_t i = 0;

    // __m256i mant_mask_v = _mm256_set1_epi32(mant_mask);
    // __m256i error_exp_v = _mm256_set1_epi32(error_exp);
    // __m256i exp_mask_v = _mm256_set1_epi32(0xFF);
    // for (; i + 8 <= sz; i += 8)
    // {
    //     __m256i v = _mm256_loadu_si256((__m256i *)&input[i]);
    //     __m256i v_exp = _mm256_and_si256(_mm256_srli_epi32(v, 23), exp_mask_v);
    //     __m256i exp_diff = _mm256_sub_epi32(v_exp, error_exp_v);
    //     __m256i v_masked = _mm256_andnot_si256(_mm256_srlv_epi32(mant_mask_v, exp_diff), v);
    //     __m256i result = _mm256_andnot_si256(_mm256_cmpgt_epi32(error_exp_v, v_exp), v_masked);
    //     _mm256_storeu_si256((__m256i *)&out[i], result);
    // }

    for (; i < sz; i++)
    {
        ieee754_float v = {input[i]};
        if (v.ieee.exponent >= error_exp)
        {
            v.ieee.mantissa &= ~(mant_mask >> (v.ieee.exponent - error_exp));
            out[i] = v.f;
        }
        else
        {
            out[i] = 0;
        }
    }

    // double magic = 0;
    // for (i = 0; i < sz; i++)
    //     magic += out[i];
    // std::cout << "Magic number: " << magic << std::endl;
}

union my_ieee754_double {
    double d;
    /* This is the IEEE 754 double-precision format.  */
    struct
    {
#if __BYTE_ORDER == __BIG_ENDIAN
#error "Unsupported endianness"
#endif /* Big endian.  */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#if __FLOAT_WORD_ORDER == __BIG_ENDIAN
#error "Unsupported endianness"
#else
        uint64_t mantissa : 52;
        uint32_t exponent : 11;
        uint32_t negative : 1;
    } ieee;
#endif
#endif /* Little endian.  */
    };

    void mask(const double *input, size_t sz, double *out, double e)
    {
        const ieee754_double error{e};
        const auto error_exp = error.ieee.exponent;
        const uint64_t mant_mask = 0xFFFFFFFFFFFFF;
        size_t i = 0;

        for (; i < sz; i++)
        {
            my_ieee754_double v = {input[i]};
            if (v.ieee.exponent >= error_exp)
            {
                v.ieee.mantissa &= ~(mant_mask >> (v.ieee.exponent - error_exp));
                out[i] = v.d;
            }
            else
            {
                out[i] = 0;
            }
        }

        // double magic = 0;
        // for (i = 0; i < sz; i++)
        //     magic += out[i];
        // std::cout << "Magic number: " << magic << std::endl;
    }

    template <typename F> size_t Mask<F>::compress(std::span<const F> input)
    {
        std::unique_ptr<F[]> out(new F[input.size()]);
        mask(&input[0], input.size(), &out[0], Method<F>::error);
        compressed_span = encoding->encode(std::as_bytes(std::span(out.get(), input.size())));
        return compressed_span.size_bytes();
    }

    template <typename F> std::span<const F> Mask<F>::decompress()
    {
        return as_typed_span<F>(encoding->decode(compressed_span));
    }
    template class Mask<float>;
    template class Mask<double>;
