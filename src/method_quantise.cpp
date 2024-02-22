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
    F prev = 0;
    for (const F v : input)
    {
        F diff = v - prev;
        int16_t index = std::round(diff / (2 * Method<F>::error));
        F reconstruction = prev + Method<F>::error * index * 2;
        if (index != std::numeric_limits<int16_t>::min() && std::abs(v - reconstruction) <= Method<F>::error)
        {
            indices.push_back(index);
            prev = reconstruction;
        }
        else
        {
            indices.push_back(std::numeric_limits<int16_t>::min());
            outliers.push_back(v);
            prev = v;
        }
    }

    std::vector<std::byte> stream;
    if constexpr (split)
    {
        auto outlier_ss = streamsplit_enc<F>(std::as_bytes(std::span(outliers)));
        auto indicies_ss = streamsplit_enc<uint16_t>(std::as_bytes(std::span(indices)));
        stream = pack_streams(std::span(outlier_ss), std::span(indicies_ss));
    }
    else
    {
        stream = pack_streams(std::span(outliers), std::span(indices));
    }
    compressed_span = encoding->encode(stream);
    return compressed_span.size_bytes();
}

template <typename F, bool split> std::span<const F> Quantise<F, split>::decompress()
{
    std::span<const std::byte> decompressed_buffer = encoding->decode(compressed_span);
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
        outliers = as_typed_span<F>(outliers_vec);
        indices = as_typed_span<int16_t>(indices_vec);
    }
    else
    {
        unpack_streams(decompressed_buffer, outliers, indices);
    }

    result.clear();
    result.reserve(indices.size());

    auto outlier = outliers.begin();
    F value = 0;
    for (const int16_t v : indices)
    {
        if (v == std::numeric_limits<int16_t>::min())
        {
            assert(outlier != outliers.end());
            value = *(outlier++);
        }
        else
        {
            value = value + Method<F>::error * v * 2;
        }
        result.push_back(value);
    }
    return result;
}
template class Quantise<float, true>;
template class Quantise<float, false>;
template class Quantise<double, true>;
template class Quantise<double, false>;