#include <cstddef>
#include <span>
#include <string>
#include <vector>

std::span<const float> as_float_span(const std::vector<std::byte> &input);

std::span<float> as_float_span(std::vector<std::byte> &input);

template <typename T> void vec_to_file(std::string path, const std::vector<T> &data);

template <typename T> std::vector<T> vec_from_file(std::string path);