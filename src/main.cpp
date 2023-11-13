#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "method.hpp"

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

template <class T>
void benchmark(const std::vector<float> &original_buffer)
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
    std::cout << "done" << std::endl
              << "Decompressing... ";
    std::cout.flush();
    tstart = std::chrono::high_resolution_clock::now();
    std::vector<float> decompressed = method.decompress();
    tend = std::chrono::high_resolution_clock::now();
    auto decompress_duration = std::chrono::duration<double>(tend - tstart);
    std::cout << "done" << std::endl
              << "Comparing... ";
    std::cout.flush();

    assert(original_buffer.size() == decompressed.size());
    double error = 0;
    for (size_t i = 0; i < original_buffer.size(); i++)
    {
        error += std::abs(decompressed[i] - original_buffer[i]);
    }
    error /= original_buffer.size();
    std::cout << "done" << std::endl;
    std::cout << "Original size:   " << original_buffer.size() * 4 << std::endl;
    std::cout << "Compressed size: " << compressed_sz << std::endl;
    std::cout << "Ratio: " << (100.0f * compressed_sz / (original_buffer.size() * 4)) << "%" << std::endl;
    std::cout << "Compression time:     " << compress_duration.count() << "s" << std::endl;
    std::cout << "Decompression time:   " << decompress_duration.count() << "s" << std::endl;
    std::cout << "Mean Absolute Error:  " << error << std::endl;
}

int main(int argc, char **argv)
{
    std::vector<float> original_buffer = generate_random_data(20000000); // 20000000);
    benchmark<Bsc>(original_buffer);
    benchmark<Sz3>(original_buffer);
    return 0;
}