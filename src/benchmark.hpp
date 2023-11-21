#pragma once
#include "method.hpp"
#include <string>
#include <vector>

std::vector<float> generate_random_data(size_t size);

template <class T> void benchmark(const std::vector<float> &original_buffer);

template <typename T> void vec_to_file(std::string path, const std::vector<T> &data);

template <typename T> std::vector<T> vec_from_file(std::string path);
