#include "SZ3/api/sz.hpp"
#include "method.hpp"

template <typename F> size_t Sz3<F>::compress(std::span<const F> input)
{
    if (input.size() < 10)
    {
        throw std::runtime_error("Sz requires at least 10 values");
    }
    SZ3::Config conf(input.size());
    conf.cmprAlgo = SZ3::ALGO_INTERP_LORENZO;
    conf.errorBoundMode = SZ3::EB_ABS;
    conf.absErrorBound = 1;
    compressed_data.reset(SZ_compress<F>(conf, input.data(), compressed_size));
    return compressed_size;
}

template <typename F> std::span<const F> Sz3<F>::decompress()
{
    SZ3::Config conf;
    decompressed_data.reset(SZ_decompress<F>(conf, compressed_data.get(), compressed_size));
    decompressed_size = conf.num;
    return std::span(decompressed_data.get(), conf.num);
}
template class Sz3<float>;
template class Sz3<double>;
