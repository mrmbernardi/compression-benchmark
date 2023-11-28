#include "benchmark.hpp"
#include "method.hpp"
#include "encoding.hpp"
#include <random>

std::vector<float> generate_random_data(size_t size)
{
    std::vector<float> data(size);
    std::mt19937 gen;
    std::uniform_real_distribution<> dis(-500, +500);
    std::cout << "Generating random floating point data... ";
    std::cout.flush();
    for (float &x : data)
    {
        x = dis(gen);
    }
    std::cout << "done." << std::endl;
    return data;
}

std::string bench_result::to_string()
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

bench_result benchmark(std::span<const float> original_buffer, Method &method)
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

    bench_result b;
    b.name = method.name();
    b.original_size = original_buffer.size() * sizeof(original_buffer[0]);
    b.compressed_size = compressed_sz;
    b.compression_time = compress_duration.count();
    b.decompression_time = decompress_duration.count();
    b.mean_absolute_error = mae;
    b.max_error = max_error;
    std::cout << b.to_string();
    std::cout.flush();
    return b;
}