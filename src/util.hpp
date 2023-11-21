#include <cstddef>
#include <span>
#include <vector>

std::span<const float> as_float_span(std::vector<std::byte> input);