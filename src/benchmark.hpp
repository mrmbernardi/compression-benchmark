#pragma once
#include "method.hpp"
#include <cstddef>
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

template <class T> bench_result benchmark(const std::vector<float> &original_buffer);
