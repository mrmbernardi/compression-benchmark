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
            return std::inner_product(signal_static.begin(), signal_static.end() - 1, w.begin(), 0);
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
        float y = std::inner_product(signal.begin(), signal.end() - 1, w.begin(), 0);
        float e = true_val - y;
        float dot = std::inner_product(signal.begin(), signal.end() - 1, signal.begin(), 0);
        float nu = mu / (eps + dot);
        for (size_t i = 0; i < N; i++)
        {
            w[i] += nu * e * signal[i];
        }
        int x = 13;
    }
};

size_t Lfzip::compress(const std::vector<float> &input)
{
    std::vector<int16_t> test = vec_from_file<int16_t>("../../LFZip/debug/bin_idx.0");

    std::vector<float> recon(input.size());
    NlmsFilter<filter_size> nlms;
    indices.clear();

    auto input_it = input.begin();
    for (std::vector<float>::iterator recon_it = recon.begin(); recon_it != recon.end(); recon_it++)
    {
        auto begin = recon_it - filter_size - 1;
        if (begin < recon.begin())
        {
            begin = recon.begin();
        }
        float prediction = nlms.predict(std::span<const float>(begin, recon_it));
        float diff = *(input_it++) - prediction;
        int64_t index = std::round((diff / (2.0 * error)));
        indices.push_back(index);
        *recon_it = prediction + index * 2.0 * error;

        if(test[indices.size() - 1] != indices.back())
        {
            assert(false);
        }
    }
    assert(indices == test);
    return 1; // nonsense for now
}

std::vector<float> Lfzip::decompress()
{
    std::vector<float> test = vec_from_file<float>("../../LFZip/debug/recon.bin");

    std::vector<float> result;
    result.reserve(indices.size());
    for (int16_t v : indices)
    {
        result.push_back(v * 2.0 * error);
    }
    assert(result == test);
    int x = 1;
    return result;
}