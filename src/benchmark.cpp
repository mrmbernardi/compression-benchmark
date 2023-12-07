#include "benchmark.hpp"
#include "encoding.hpp"
#include "method.hpp"
#include <cstddef>
#include <random>

template <typename F> std::vector<F> generate_random_data(size_t size)
{
    std::vector<F> data(size);
    std::mt19937 gen;
    std::uniform_real_distribution<F> dis(-500, +500);
    std::cout << "Generating random floating point data... ";
    std::cout.flush();
    for (F &x : data)
    {
        x = dis(gen);
    }
    std::cout << "done." << std::endl;
    return data;
}
template std::vector<float> generate_random_data(size_t size);
template std::vector<double> generate_random_data(size_t size);

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
bench_result_ex benchmark(std::span<const F> original_buffer, Method<F> &method, std::span<F> output_buffer)
{
    std::cout << std::endl;
    std::cout << "Using method " << method.name() << std::endl;
    std::cout << "Compressing... ";
    std::cout.flush();
    auto tstart = std::chrono::high_resolution_clock::now();
    size_t compressed_sz = method.compress(original_buffer);
    auto tend = std::chrono::high_resolution_clock::now();
    auto compress_duration = std::chrono::duration<double>(tend - tstart);
    std::cout << "done" << std::endl << "Decompressing... ";
    std::cout.flush();
    tstart = std::chrono::high_resolution_clock::now();
    auto decompressed = method.decompress();
    tend = std::chrono::high_resolution_clock::now();
    auto decompress_duration = std::chrono::duration<double>(tend - tstart);
    std::cout << "done" << std::endl << "Comparing... ";
    std::cout.flush();

    assert(original_buffer.size() == decompressed.size());

    double mae = 0;
    double max_error = 0;
    for (size_t i = 0; i < original_buffer.size(); i++)
    {
        double e = std::abs(decompressed[i] - original_buffer[i]);
        mae += e;
        max_error = std::max(max_error, e);
    }
    mae /= original_buffer.size();
    std::cout << "done" << std::endl;

    bench_result_ex b;
    b.name = method.name();
    b.original_size = original_buffer.size() * sizeof(original_buffer[0]);
    b.compressed_size = compressed_sz;
    b.compression_time = compress_duration.count();
    b.decompression_time = decompress_duration.count();
    b.mean_absolute_error = mae;
    b.max_error = max_error;
    std::cout << b.to_string();
    std::cout.flush();

    if (decompressed.size() == output_buffer.size())
    {
        std::copy(decompressed.begin(), decompressed.end(), output_buffer.begin());
    }

    return b;
}
template bench_result_ex benchmark(std::span<const float> original_buffer, Method<float> &method,
                                   std::span<float> output_buffer);
template bench_result_ex benchmark(std::span<const double> original_buffer, Method<double> &method,
                                   std::span<double> output_buffer);

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