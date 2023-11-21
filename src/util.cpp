#include "util.hpp"
#include <cassert>

std::span<const float> as_float_span(std::vector<std::byte> input)
{
    assert(input.size() % sizeof(float) == 0);
    auto data_ptr = reinterpret_cast<float *>(input.data());
    return std::span<const float>(data_ptr, input.size() / sizeof(float));
}