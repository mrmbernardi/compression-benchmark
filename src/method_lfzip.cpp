#include "benchmark.hpp"
#include "method.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
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
    std::vector<int16_t> test = vec_from_file<int16_t>("../../LFZip/debug/bin_idx.0");

    std::vector<float> recon;
    recon.reserve(input.size());
    NlmsFilter<filter_size> nlms;
    indices.clear();

    for (const float v : input)
    {
        auto sample_start = std::max(recon.end() - filter_size - 1, recon.begin());
        float predval = nlms.predict(std::span<const float>(sample_start, recon.end()));
        float diff = v - predval;
        int64_t index = std::round(diff / (2.0 * error));
        // TODO: Handle out of range values.
        indices.push_back(index);
        recon.push_back(predval + error * index * 2.0);
    }
    assert(indices == test);
    return 1; // TODO: nonsense for now
}

std::vector<float> Lfzip::decompress()
{
    std::vector<float> test = vec_from_file<float>("../../LFZip/debug/recon.bin");

    std::vector<float> result;
    result.reserve(indices.size());
    NlmsFilter<filter_size> nlms;
    for (const int16_t v : indices)
    {
        auto sample_start = std::max(result.end() - filter_size - 1, result.begin());
        float predval = nlms.predict(std::span<const float>(sample_start, result.end()));
        result.push_back(predval + error * v * 2.0);
    }
    assert(result == test);
    return result;
}