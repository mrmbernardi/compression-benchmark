#include "benchmark.hpp"
#include "method.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
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
    std::cout << "Original size:   " << original_buffer.size() * 4 << std::endl;
    std::cout << "Compressed size: " << compressed_sz << std::endl;
    std::cout << "Ratio: " << (100.0f * compressed_sz / (original_buffer.size() * 4)) << "%" << std::endl;
    std::cout << "Compression time:     " << compress_duration.count() << "s" << std::endl;
    std::cout << "Decompression time:   " << decompress_duration.count() << "s" << std::endl;
    std::cout << "Mean Absolute Error:  " << error << std::endl;
}
template void benchmark<Bsc>(const std::vector<float> &original_buffer);
template void benchmark<Sz3>(const std::vector<float> &original_buffer);
template void benchmark<Lfzip>(const std::vector<float> &original_buffer);

template <typename T> void vec_to_file(std::string path, const std::vector<T> &data)
{
    if (std::FILE *f = std::fopen(path.c_str(), "wb"))
    {
        std::fwrite(data.data(), sizeof(data[0]), data.size(), f);
        std::fclose(f);
    }
    else
    {
        throw std::runtime_error("cannot open file");
    }
}
template void vec_to_file(std::string path, const std::vector<float> &data);
template void vec_to_file(std::string path, const std::vector<uint16_t> &data);

template <typename T> std::vector<T> vec_from_file(std::string path)
{
    std::vector<T> data;
    if (std::FILE *f = std::fopen(path.c_str(), "rb"))
    {
        T v;
        while (std::fread(&v, sizeof(v), 1, f))
        {
            data.push_back(v);
        }
        std::fclose(f);
    }
    else
    {
        throw std::runtime_error("cannot open file");
    }
    return data;
}
template std::vector<float> vec_from_file(std::string path);
template std::vector<int16_t> vec_from_file(std::string path);