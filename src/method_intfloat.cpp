#include "method.hpp"
#include "util.hpp"
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <valarray>

// template <typename  F> constexpr F get_greatest_precise_int();
// template <> constexpr double get_greatest_precise_int()
// {
//     return (double)(1ull << 53);
// }
// template <> constexpr float get_greatest_precise_int()
// {
//     return (float)(1 << 24);
// }
// template <typename F> constexpr uint64_t get_midpoint()
// template <double> constexpr uint64_t get_midpoint()
// {
//     return 1ull << 63;
// }
// template <float> constexpr uint32_t get_midpoint()
// {
//     return 1 << 31;
// }

template <typename F> struct consts;
template <> struct consts<double>
{
    static constexpr double greatest_precise_int = (double)(1ull << 53);
    static constexpr uint64_t midpoint = 1ull << 63;
    typedef uint64_t uint_type;
};
template <> struct consts<float>
{
    static constexpr float greatest_precise_int = (float)(1 << 24);
    static constexpr uint32_t midpoint = 1 << 31;
    typedef uint32_t uint_type;
};

template <typename F> void to_uint(const F *input, size_t sz, F *out, F e)
{
    /*
    fn from_int_float(float: Self::Float) -> Self {
        let abs = float.abs();
        let gpi = <$float>::GREATEST_PRECISE_INT;
        let abs_int = if abs < gpi {
          abs as Self
        } else {
          gpi as Self + (abs.to_bits() - gpi.to_bits())
        };
        if float.is_sign_positive() {
          Self::MID + abs_int
        } else {
          // -1 because we need to distinguish -0.0 from +0.0
          Self::MID - 1 - abs_int
        }
    }
    */
    F inv_error = ((F)1) / e;
    for (size_t i = 0; i < sz; i++)
    {
        F abs = std::abs(input[i] * inv_error);
        F gpi = consts<F>::greatest_precise_int;
        typename consts<F>::uint_type abs_int;
        if (abs < gpi)
        {
            abs_int = abs;
        }
        else
        {
            auto abs_bits = std::bit_cast<typename consts<F>::uint_type>(abs);
            auto gpi_bits = std::bit_cast<typename consts<F>::uint_type>(consts<F>::greatest_precise_int);
            abs_int = ((typename consts<F>::uint_type)gpi) + (abs_bits - gpi_bits);
        }
        if (!std::signbit(input[i]))
        {
            out[i] = std::bit_cast<F>(consts<F>::midpoint + abs_int);
        }
        else
        {
            out[i] = std::bit_cast<F>(consts<F>::midpoint - 1 - abs_int);
        }
    }
}

template <typename F> void from_uint(const F *input, size_t sz, F *out, F e)
{
    /*
    fn to_int_float(self) -> Self::Float {
        let (negative, abs_int) = if self >= Self::MID {
          (false, self - Self::MID)
        } else {
          (true, Self::MID - 1 - self)
        };
        let gpi = <$float>::GREATEST_PRECISE_INT;
        let abs_float = if abs_int < gpi as Self {
          abs_int as $float
        } else {
          <$float>::from_bits(gpi.to_bits() + (abs_int - gpi as Self))
        };
        if negative {
          -abs_float
        } else {
          abs_float
        }
    }const From &from
    */
    for (size_t i = 0; i < sz; i++)
    {
        auto v_int = std::bit_cast<typename consts<F>::uint_type>(input[i]);
        bool negative;
        typename consts<F>::uint_type abs_int;
        if (v_int >= consts<F>::midpoint)
        {
            negative = false;
            abs_int = v_int - consts<F>::midpoint;
        }
        else
        {
            negative = true;
            abs_int = consts<F>::midpoint - 1 - v_int;
        }
        typename consts<F>::uint_type gpi = consts<F>::greatest_precise_int;
        F abs_float;
        if (abs_int < gpi)
        {
            abs_float = abs_int;
        }
        else
        {
            auto gpi_bits = std::bit_cast<typename consts<F>::uint_type>(consts<F>::greatest_precise_int);
            abs_float = std::bit_cast<F>(gpi_bits + (abs_int - gpi));
        }
        if (negative)
        {
            out[i] = -abs_float * e;
        }
        else
        {
            out[i] = abs_float * e;
        }
    }
}

template <typename F> size_t IntFloat<F>::compress(std::span<const F> input)
{
    std::unique_ptr<F[]> out(new F[input.size()]);
    to_uint(input.data(), input.size(), out.get(), Method<F>::error);
    compressed_span = encoding->encode(std::as_bytes(std::span(out.get(), input.size())));
    return compressed_span.size_bytes();
}

template <typename F> std::span<const F> IntFloat<F>::decompress()
{
    std::span<const std::byte> decoded = encoding->decode(compressed_span);
    const F *input = reinterpret_cast<const F *>(decoded.data());
    const size_t outputSz = decoded.size_bytes() / sizeof(F);
    results.reset(new F[outputSz]);
    from_uint(input, outputSz, results.get(), Method<F>::error);
    return std::span<const F>(results.get(), outputSz);
}
template class IntFloat<float>;
template class IntFloat<double>;
