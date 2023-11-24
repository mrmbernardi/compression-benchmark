#pragma once
#include "method.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<float> generate_random_data(size_t size);

struct bench_result
{
    std::string name;
    size_t original_size;
    size_t compressed_size;
    double compression_time;
    double decompression_time;
    double max_error;
    double mean_absolute_error;

    std::string to_string();
};

template <class T> bench_result benchmark(const std::vector<float> &original_buffer)
{
    T method;
    std::cout << std::endl;
    std::cout << "Using method " << method.name << std::endl;
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
    b.name = method.name;
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