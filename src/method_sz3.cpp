#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <SZ3/api/sz.hpp>
#pragma clang diagnostic pop
#include "method.hpp"

void Sz3::clear_data()
{
    if (compressed_data)
    {
        delete[] compressed_data;
        compressed_data = nullptr;
    }
    if (decompressed_data)
    {
        delete[] decompressed_data;
        decompressed_data = nullptr;
    }
}

size_t Sz3::compress(const std::vector<float> &input)
{
    clear_data();
    SZ3::Config conf(input.size());
    conf.cmprAlgo = SZ3::ALGO_INTERP_LORENZO;
    conf.errorBoundMode = SZ3::EB_ABS;
    conf.absErrorBound = 1;
    compressed_data = SZ_compress<float>(conf, input.data(), compressed_size);
    return compressed_size;
}

std::span<const float> Sz3::decompress()
{
    SZ3::Config conf;
    decompressed_data = SZ_decompress<float>(conf, compressed_data, compressed_size);
    decompressed_size = conf.num;
    return std::span(decompressed_data, conf.num);
}

Sz3::~Sz3()
{
    clear_data();
}
