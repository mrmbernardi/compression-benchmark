#include "benchmark.hpp"
#include "method.hpp"
#include "util.hpp"
#include "encoding.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <span>
#include <vector>

template <size_t N> class NlmsFilter
{
    static constexpr float mu = 0.5;
    static constexpr float eps = 1.0;
    std::array<float, N> w = {};

  public:
    float predict(std::span<const float> signal)
    {
        assert(signal.size() <= N + 1);
        if (signal.size() == N + 1)
        {
            auto signal_static = signal.subspan<0, N + 1>();
            adapt(signal_static);
            return std::inner_product(signal_static.begin() + 1, signal_static.end(), w.begin(), 0.0f);
        }
        else if (signal.size() == 0)
        {
            return 0.0f;
        }
        else
        {
            return signal.back();
        }
    }
    void adapt(std::span<const float, N + 1> signal)
    {
        float true_val = signal.back();
        float y = std::inner_product(signal.begin(), signal.end() - 1, w.begin(), 0.0f);
        float e = true_val - y;
        float dot = std::inner_product(signal.begin(), signal.end() - 1, signal.begin(), 0.0f);
        float nu = mu / (eps + dot);
        for (size_t i = 0; i < N; i++)
        {
            w[i] += nu * e * signal[i];
        }
    }
};

size_t Lfzip::compress(const std::vector<float> &input)
{
    // std::vector<int16_t> test = vec_from_file<int16_t>("../../LFZip/debug/bin_idx.0");

    NlmsFilter<filter_size> nlms;
    std::vector<int16_t> indices;
    std::vector<float> outliers;
    std::vector<float> recon;
    recon.reserve(input.size());
    indices.reserve(input.size());

    for (const float v : input)
    {
        auto sample_start = std::max(recon.end() - filter_size - 1, recon.begin());
        float predval = nlms.predict(std::span<const float>(sample_start, recon.end()));
        float diff = v - predval;
        int16_t index = std::round(diff / (2.0 * error));
        float reconstruction = predval + error * index * 2.0;
        if (std::abs(v - reconstruction) > maxerror_original || index == std::numeric_limits<int16_t>::min())
        {
            indices.push_back(std::numeric_limits<int16_t>::min());
            outliers.push_back(v);
            recon.push_back(v);
        }
        else
        {
            indices.push_back(index);
            recon.push_back(reconstruction);
        }
    }

    // assert(indices == test);

    auto stream = pack_streams(outliers, indices);
    // std::cout << "Lfzip before compression: " << stream.size() << std::endl;
    compressed_buffer = encoding.encode(stream);
    // std::cout << "Lfzip after compression: " << compressed_buffer.size() << std::endl;
    return compressed_buffer.size();
}

std::vector<float> Lfzip::decompress()
{
    // std::vector<float> test = vec_from_file<float>("../../LFZip/debug/recon.bin");

    std::vector<std::byte> decompressed_buffer = encoding.decode(compressed_buffer);
    std::span<const float> outliers;
    std::span<const int16_t> indices;
    unpack_streams(decompressed_buffer, outliers, indices);

    std::vector<float> result;
    result.reserve(indices.size());
    NlmsFilter<filter_size> nlms;
    auto outlier = outliers.begin();
    for (const int16_t v : indices)
    {
        auto sample_start = std::max(result.end() - filter_size - 1, result.begin());
        float predval = nlms.predict(std::span<const float>(sample_start, result.end()));
        if (v == std::numeric_limits<int16_t>::min())
        {
            assert(outlier != outliers.end());
            result.push_back(*(outlier++));
        }
        else
        {
            result.push_back(predval + error * v * 2.0);
        }
    }
    // std::cout << "magic sum: " << std::accumulate(result.begin(), result.end(), 0.0L) << std::endl;
    // assert(result == test);
    return result;
}