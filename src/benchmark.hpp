#pragma once
#include "method.hpp"
#include <string>
#include <vector>

std::vector<float> generate_random_data(size_t size);

template <class T> void benchmark(const std::vector<float> &original_buffer);
