#include "benchmark.hpp"
#include "method.hpp"
#include "wrapper.hpp"
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
    const float mu = 0.5;
    const float eps = 1.0;
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

template <class LosslessWrapper> size_t Lfzip<LosslessWrapper>::compress(const std::vector<float> &input)
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
            recon.push_back(predval + error * index * 2.0);
        }
    }

    // assert(indices == test);

    std::vector<std::byte> stream;
    size_t indices_stream_size = indices.size();
    auto s1 = std::as_bytes(std::span<size_t, 1>(&indices_stream_size, 1));
    auto s2 = std::as_bytes(std::span(indices));
    auto s3 = std::as_bytes(std::span(outliers));
    stream.reserve(s1.size_bytes() + s2.size_bytes() + s3.size_bytes());
    stream.insert(stream.end(), s1.begin(), s1.end());
    stream.insert(stream.end(), s2.begin(), s2.end());
    stream.insert(stream.end(), s3.begin(), s3.end());

    // std::cout << "Lfzip before compression: " << stream.size() << std::endl;
    compressed_buffer = LosslessWrapper::compress(stream);
    // std::cout << "Lfzip after compression: " << compressed_buffer.size() << std::endl;
    return compressed_buffer.size();
}

template <class LosslessWrapper> std::vector<float> Lfzip<LosslessWrapper>::decompress()
{
    // std::vector<float> test = vec_from_file<float>("../../LFZip/debug/recon.bin");

    std::vector<std::byte> decompressed_buffer = LosslessWrapper::decompress(compressed_buffer);
    size_t indicies_count = *reinterpret_cast<const size_t *>(decompressed_buffer.data());
    auto indices_start = decompressed_buffer.data() + sizeof(size_t);
    auto outliers_start = indices_start + sizeof(int16_t) * indicies_count;
    auto outliers_end = reinterpret_cast<const float *>(decompressed_buffer.data() + decompressed_buffer.size());
    auto indices = std::span<const int16_t>(reinterpret_cast<const int16_t *>(indices_start), indicies_count);
    auto outliers = std::span<const float>(reinterpret_cast<const float *>(outliers_start), outliers_end);

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
    // assert(result == test);
    return result;
}

template class Lfzip<Bsc>;
template class Lfzip<Zstd>;