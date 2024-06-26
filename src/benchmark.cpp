#include "benchmark.hpp"
#include "method.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <random>
#include <sstream>

template <typename F> std::vector<F> generate_random_data(size_t size, double lower, double upper, bool seed)
{
    std::vector<F> data(size);
    std::mt19937 gen;
    if(seed)
    {
        gen = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }
    std::uniform_real_distribution<F> dis(lower, upper);
    std::cout << "Generating random floating point data... ";
    std::cout.flush();
    for (F &x : data)
    {
        x = dis(gen);
    }
    std::cout << "done." << std::endl;
    return data;
}
template std::vector<float> generate_random_data(size_t size, double lower, double upper, bool seed);
template std::vector<double> generate_random_data(size_t size, double lower, double upper, bool seed);

std::string bench_result_ex::to_string()
{
    std::ostringstream s;
    s << "Original size:        " << original_size << std::endl;
    s << "Compressed size:      " << compressed_size << std::endl;
    s << "Ratio:                " << ((100.0f * compressed_size) / original_size) << "%" << std::endl;
    s << "Compression time:     " << compression_time << "s" << std::endl;
    s << "Decompression time:   " << decompression_time << "s" << std::endl;
    s << "Mean Absolute Error:  " << mean_absolute_error << std::endl;
    s << "Max Error:            " << max_error << std::endl;
    return s.str();
}

template <typename F>
bench_result_ex benchmark(std::span<const F> original_buffer, Method<F> &method, F error_bound,
                          std::span<F> output_buffer, bool quiet, bool skip_metrics)
{
    if (!quiet)
    {
        std::cout << std::endl;
        std::cout << "Using method " << method.name() << std::endl;
        std::cout << "Compressing... ";
        std::cout.flush();
    }
    method.set_error_bound(error_bound);
    auto tstart = std::chrono::high_resolution_clock::now();
    size_t compressed_sz = method.compress(original_buffer);
    auto tend = std::chrono::high_resolution_clock::now();
    auto compress_duration = std::chrono::duration<double>(tend - tstart);
    if (!quiet)
    {
        std::cout << "done" << std::endl << "Decompressing... ";
        std::cout.flush();
    }
    tstart = std::chrono::high_resolution_clock::now();
    auto decompressed = method.decompress();
    tend = std::chrono::high_resolution_clock::now();
    auto decompress_duration = std::chrono::duration<double>(tend - tstart);
    if (!quiet)
    {
        std::cout << "done" << std::endl;
        std::cout.flush();
    }

    assert(original_buffer.size() == decompressed.size());

    double mae = 0;
    double max_error = 0;

    if (!skip_metrics)
    {
        if (!quiet)
        {
            std::cout << "Comparing... ";
        }
        for (size_t i = 0; i < original_buffer.size(); i++)
        {
            double a, b;
            a = decompressed[i];
            b = original_buffer[i];
            if(std::isnan(a) && std::isnan(b)) continue;

            double e = std::abs(a - b);
            assert(e <= error_bound);
            mae += e;
            max_error = std::max(max_error, e);
        }
        mae /= original_buffer.size();
        if (!quiet)
        {
            std::cout << "done" << std::endl;
        }
    }

    bench_result_ex b;
    b.name = method.name();
    b.original_size = original_buffer.size() * sizeof(original_buffer[0]);
    b.compressed_size = compressed_sz;
    b.compression_time = compress_duration.count();
    b.decompression_time = decompress_duration.count();
    b.mean_absolute_error = mae;
    b.max_error = max_error;
    if (!quiet)
    {
        std::cout << b.to_string();
        std::cout.flush();
    }

    if (decompressed.size() == output_buffer.size())
    {
        std::copy(decompressed.begin(), decompressed.end(), output_buffer.begin());
    }

    return b;
}
template bench_result_ex benchmark(std::span<const float> original_buffer, Method<float> &method, float error_bound,
                                   std::span<float> output_buffer, bool quiet, bool skip_metrics);
template bench_result_ex benchmark(std::span<const double> original_buffer, Method<double> &method, double error_bound,
                                   std::span<double> output_buffer, bool quiet, bool skip_metrics);

bench_result &bench_result::operator=(const bench_result_ex &other)
{
    original_size = other.original_size;
    compressed_size = other.compressed_size;
    compression_time = other.compression_time;
    decompression_time = other.decompression_time;
    max_error = other.max_error;
    mean_absolute_error = other.mean_absolute_error;
    return *this;
}