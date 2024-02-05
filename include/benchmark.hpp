#pragma once
#include <span>
#include <string>
#include <vector>

template <typename F> class Method;

template <typename F> std::vector<F> generate_random_data(size_t size);

struct bench_result_ex;
struct bench_result
{
    size_t original_size;
    size_t compressed_size;
    double compression_time;
    double decompression_time;
    double max_error;
    double mean_absolute_error;

    bench_result &operator=(const bench_result_ex &other);
};

struct bench_result_ex : bench_result
{
    std::string name;
    double mbytes()
    {
        return (double)(original_size) / (1024.0l * 1024.0l);
    };
    double compression_data_rate()
    {
        return mbytes() / compression_time;
    }
    double decompression_data_rate()
    {
        return mbytes() / decompression_time;
    }

    std::string to_string();
};

template <typename F>
bench_result_ex benchmark(std::span<const F> original_buffer, Method<F> &method, F error_bound = 1.0,
                          std::span<F> output_buffer = std::span<F>(), bool quiet = false, bool skip_metrics = false);