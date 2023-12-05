#include "util.hpp"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>

void table_to_file(std::string path, tabulate::Table &table)
{
    std::ofstream csv;
    csv.open(path);
    if (csv.is_open())
    {
        for (auto &row : table)
        {
            bool first = true;
            for (auto &cell : row)
            {
                if (!first)
                    csv << ", ";
                else
                    first = false;
                csv << cell.get_text();
            }
            csv << '\n';
        }
        csv.close();
    }
    else
    {
        throw std::runtime_error("cannot open file");
    }
}

template <typename F> std::span<const F> as_float_span(const std::vector<std::byte> &input)
{
    assert(input.size() % sizeof(F) == 0);
    auto data_ptr = reinterpret_cast<const F *>(input.data());
    return std::span<const F>(data_ptr, input.size() / sizeof(F));
}
template std::span<const float> as_float_span(const std::vector<std::byte> &input);
template std::span<const double> as_float_span(const std::vector<std::byte> &input);

template <typename F> std::span<F> as_float_span(std::vector<std::byte> &input)
{
    assert(input.size() % sizeof(F) == 0);
    auto data_ptr = reinterpret_cast<F *>(input.data());
    return std::span<F>(data_ptr, input.size() / sizeof(F));
}
template std::span<float> as_float_span(std::vector<std::byte> &input);
template std::span<double> as_float_span(std::vector<std::byte> &input);

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
template void vec_to_file(std::string path, const std::vector<uint16_t> &data);
template void vec_to_file(std::string path, const std::vector<float> &data);
template void vec_to_file(std::string path, const std::vector<double> &data);

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
template std::vector<int16_t> vec_from_file(std::string path);
template std::vector<float> vec_from_file(std::string path);
template std::vector<double> vec_from_file(std::string path);