#include "encoding.hpp"
#include "method.hpp"
#include "util.hpp"
#include <cmath>
#include <limits>
#include <vector>

template <typename F> size_t Quantise<F>::compress(std::span<const F> input)
{
    std::vector<F> outliers;
    std::vector<int16_t> indices;
    indices.reserve(input.size());
    F prev = 0.0f;
    for (const F v : input)
    {
        F diff = v - prev;
        int16_t index = std::round(diff / (2.0 * error));
        F reconstruction = prev + error * index * 2.0;
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
    compressed_buffer = encoding->encode(pack_streams(outliers, indices));
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

template <typename F> std::span<const F> Quantise<F>::decompress()
{
    std::vector<std::byte> decompressed_buffer = encoding->decode(compressed_buffer);
    std::span<const F> outliers;
    std::span<const int16_t> indices;
    unpack_streams(decompressed_buffer, outliers, indices);

    result.clear();
    result.reserve(indices.size());

    auto outlier = outliers.begin();
    F value = 0.0f;
    for (const int16_t v : indices)
    {
        if (v == std::numeric_limits<int16_t>::min())
        {
            assert(outlier != outliers.end());
            value = *(outlier++);
        }
        else
        {
            value = value + error * v * 2.0;
        }
        result.push_back(value);
    }
    return result;
}
template class Quantise<float>;
template class Quantise<double>;