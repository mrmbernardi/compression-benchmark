#include <stddef.h>
#include <vector>
#include <libbsc.h>
#include <SZ3/api/sz.hpp>
#include "method.hpp"

void Sz3::clear_compressed_data()
{
    if (compressedData)
    {
        delete[] compressedData;
        compressedData = nullptr;
    }
}

size_t Sz3::compress(const std::vector<float> &input)
{
    clear_compressed_data();
    SZ::Config conf(input.size());
    conf.cmprAlgo = SZ::ALGO_INTERP_LORENZO;
    conf.errorBoundMode = SZ::EB_ABS;
    conf.absErrorBound = 1;
    compressedData = SZ_compress<float>(conf, input.data(), compressedSize);
    return compressedSize;
}

std::vector<float> Sz3::decompress()
{
    SZ::Config conf;
    float *decompressedData = SZ_decompress<float>(conf, compressedData, compressedSize);

    std::vector<float> reconstructed_data(decompressedData, decompressedData + conf.num);
    clear_compressed_data();
    delete[] decompressedData;
    return reconstructed_data;
}

Sz3::~Sz3()
{
    clear_compressed_data();
}