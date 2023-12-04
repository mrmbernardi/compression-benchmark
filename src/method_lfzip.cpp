#include "benchmark.hpp"
#include "encoding.hpp"
#include "method.hpp"
#include "util.hpp"
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

template <typename F, size_t N> class NlmsFilter
{
    static constexpr F mu = 0.5;
    static constexpr F eps = 1.0;
    std::array<F, N> w = {};

  public:
    F predict(std::span<const F> signal)
    {
        assert(signal.size() <= N + 1);
        if (signal.size() == N + 1)
        {
            auto signal_static = signal.template subspan<0, N + 1>();
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
    void adapt(std::span<const F, N + 1> signal)
    {
        F true_val = signal.back();
        F y = std::inner_product(signal.begin(), signal.end() - 1, w.begin(), 0.0f);
        F e = true_val - y;
        F dot = std::inner_product(signal.begin(), signal.end() - 1, signal.begin(), 0.0f);
        F nu = mu / (eps + dot);
        for (size_t i = 0; i < N; i++)
        {
            w[i] += nu * e * signal[i];
        }
    }
};

template <typename F> size_t Lfzip<F>::compress(std::span<const F> input)
{
    // std::vector<int16_t> test = vec_from_file<int16_t>("../../LFZip/debug/bin_idx.0");

    NlmsFilter<F, filter_size> nlms;
    std::vector<int16_t> indices;
    std::vector<F> outliers;
    std::vector<F> recon;
    recon.reserve(input.size());
    indices.reserve(input.size());

    for (const F v : input)
    {
        auto sample_start = std::max(recon.end() - filter_size - 1, recon.begin());
        F predval = nlms.predict(std::span<const F>(sample_start, recon.end()));
        F diff = v - predval;
        int16_t index = std::round(diff / (2.0 * error));
        F reconstruction = predval + error * index * 2.0;
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
    compressed_buffer = encoding->encode(stream);
    // std::cout << "Lfzip after compression: " << compressed_buffer.size() << std::endl;
    return compressed_buffer.size();
}

template <typename F> std::span<const F> Lfzip<F>::decompress()
{
    // std::vector<F> test = vec_from_file<F>("../../LFZip/debug/recon.bin");

    std::vector<std::byte> decompressed_buffer = encoding->decode(compressed_buffer);
    std::span<const F> outliers;
    std::span<const int16_t> indices;
    unpack_streams(decompressed_buffer, outliers, indices);

    result.clear();
    result.reserve(indices.size());
    NlmsFilter<F, filter_size> nlms;
    auto outlier = outliers.begin();
    for (const int16_t v : indices)
    {
        auto sample_start = std::max(result.end() - filter_size - 1, result.begin());
        F predval = nlms.predict(std::span<const F>(sample_start, result.end()));
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
template class Lfzip<float>;
template class Lfzip<double>;