#include "benchmark.hpp"
#include "method.hpp"
#include "wrapper.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

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

template <class T> void benchmark(const std::vector<float> &original_buffer)
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

    double error = 0;
    for (size_t i = 0; i < original_buffer.size(); i++)
    {
        error += std::abs(decompressed[i] - original_buffer[i]);
    }
    error /= original_buffer.size();
    std::cout << "done" << std::endl;
    auto original_sz = original_buffer.size() * sizeof(original_buffer[0]);
    std::cout << "Original size:   " << original_sz << std::endl;
    std::cout << "Compressed size: " << compressed_sz << std::endl;
    std::cout << "Ratio: " << (100.0f * compressed_sz / (original_sz)) << "%" << std::endl;
    std::cout << "Compression time:     " << compress_duration.count() << "s" << std::endl;
    std::cout << "Decompression time:   " << decompress_duration.count() << "s" << std::endl;
    std::cout << "Mean Absolute Error:  " << error << std::endl;
}
template void benchmark<Lossless<Bsc>>(const std::vector<float> &original_buffer);
template void benchmark<Lossless<Zstd>>(const std::vector<float> &original_buffer);
template void benchmark<Lfzip<Bsc>>(const std::vector<float> &original_buffer);
template void benchmark<Lfzip<Zstd>>(const std::vector<float> &original_buffer);
template void benchmark<Sz3>(const std::vector<float> &original_buffer);
