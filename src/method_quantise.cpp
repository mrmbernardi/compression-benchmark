#include "encoding.hpp"
#include "method.hpp"
#include "util.hpp"
#include <cmath>
#include <limits>
#include <vector>

template <typename F, bool split> size_t Quantise<F, split>::compress(std::span<const F> input)
{
    std::vector<F> outliers;
    std::vector<int16_t> indices;
    indices.reserve(input.size());
    F prev = 0.0f;
    for (const F v : input)
    {
        F diff = v - prev;
        int16_t index = std::round(diff / (2.0 * Method<F>::error));
        F reconstruction = prev + Method<F>::error * index * 2.0;
        if (std::abs(v - reconstruction) > Method<F>::error || index == std::numeric_limits<int16_t>::min())
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

    std::vector<std::byte> stream;
    if constexpr (split)
    {
        stream = pack_streams(streamsplit_enc<F>(std::as_bytes(std::span(outliers))),
                              streamsplit_enc<uint16_t>(std::as_bytes(std::span(indices))));
    }
    else
    {
        stream = pack_streams(outliers, indices);
    }
    compressed_buffer = encoding->encode(stream);
    return compressed_buffer.size() * sizeof(compressed_buffer[0]);
}

template <typename F, bool split> std::span<const F> Quantise<F, split>::decompress()
{
    std::vector<std::byte> decompressed_buffer = encoding->decode(compressed_buffer);
    std::span<const F> outliers;
    std::span<const int16_t> indices;

    std::vector<std::byte> outliers_vec;
    std::vector<std::byte> indices_vec;
    if constexpr (split)
    {
        std::span<const std::byte> outliers_tmp;
        std::span<const std::byte> indices_tmp;
        unpack_streams(decompressed_buffer, outliers_tmp, indices_tmp);
        outliers_vec = streamsplit_dec<F>(outliers_tmp);
        indices_vec = streamsplit_dec<uint16_t>(indices_tmp);
        outliers = as_span<F>(outliers_vec);
        indices = as_span<int16_t>(indices_vec);
    }
    else
    {
        unpack_streams(decompressed_buffer, outliers, indices);
    }

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
            value = value + Method<F>::error * v * 2.0;
        }
        result.push_back(value);
    }
    return result;
}
template class Quantise<float, true>;
template class Quantise<float, false>;
template class Quantise<double, true>;
template class Quantise<double, false>;