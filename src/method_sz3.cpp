#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <SZ3/api/sz.hpp>
#pragma clang diagnostic pop
#include "method.hpp"

size_t Sz3::compress(const std::vector<float> &input)
{
    SZ3::Config conf(input.size());
    conf.cmprAlgo = SZ3::ALGO_INTERP_LORENZO;
    conf.errorBoundMode = SZ3::EB_ABS;
    conf.absErrorBound = 1;
    compressed_data.reset(SZ_compress<float>(conf, input.data(), compressed_size));
    return compressed_size;
}

std::span<const float> Sz3::decompress()
{
    SZ3::Config conf;
    decompressed_data.reset(SZ_decompress<float>(conf, compressed_data.get(), compressed_size));
    decompressed_size = conf.num;
    return std::span(decompressed_data.get(), conf.num);
}
