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

bench_result benchmark(std::span<const float> original_buffer, Method &method);