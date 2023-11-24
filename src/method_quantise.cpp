#include "method.hpp"
#include "util.hpp"
#include "wrapper.hpp"
#include <cmath>
#include <limits>
#include <vector>

template <class LosslessWrapper> size_t Quantise<LosslessWrapper>::compress(const std::vector<float> &input)
{
    std::vector<float> outliers;
    std::vector<int16_t> indices;
    indices.reserve(input.size());
    float prev = 0.0f;
    for (const float v : input)
    {
        float diff = v - prev;
        int16_t index = std::round(diff / (2.0 * error));
        float reconstruction = prev + error * index * 2.0;
        if (std::abs(v - reconstruction) > error || index == std::numeric_limits<int16_t>::min())
        {
            indices.push_back(std::numeric_limits<int16_t>::min());
            outliers.push_back(v);
            prev = v;
        }
        else
        {
            indices.push_back(index);
            prev = reconstruction;
        }
    }
    compressed_buffer = LosslessWrapper::compress(pack_streams(outliers, indices));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

template <class LosslessWrapper> std::vector<float> Quantise<LosslessWrapper>::decompress()
{
    std::vector<std::byte> decompressed_buffer = LosslessWrapper::decompress(compressed_buffer);
    std::span<const float> outliers;
    std::span<const int16_t> indices;
    unpack_streams(decompressed_buffer, outliers, indices);

    std::vector<float> result;
    result.reserve(indices.size());

    auto outlier = outliers.begin();
    for (const int16_t v : indices)
    {
        if (v == std::numeric_limits<int16_t>::min())
        {
            assert(outlier != outliers.end());
            result.push_back(*(outlier++));
        }
        else
        {
            result.push_back(result.back() + error * v * 2.0);
        }
    }
    return result;
}

template class Quantise<Bsc>;
template class Quantise<Zstd>;