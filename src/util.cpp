#include "util.hpp"
#include <cassert>
#include <cstdint>
#include <stdexcept>

std::span<const float> as_float_span(std::vector<std::byte> input)
{
    assert(input.size() % sizeof(float) == 0);
    auto data_ptr = reinterpret_cast<float *>(input.data());
    return std::span<const float>(data_ptr, input.size() / sizeof(float));
}

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