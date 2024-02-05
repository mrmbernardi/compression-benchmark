#include "util.hpp"
#include "encoding.hpp"
#include "method.hpp"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

template <typename F> std::vector<std::shared_ptr<Method<F>>> get_all_common_methods()
{
    std::vector<std::shared_ptr<Encoding>> encodings;
    encodings.emplace_back(std::make_shared<Bsc>());
    encodings.emplace_back(std::make_shared<Zstd>());
    encodings.emplace_back(std::make_shared<Lz4>());
    encodings.emplace_back(std::make_shared<Compose<StreamSplit<F>, Bsc>>());
    encodings.emplace_back(std::make_shared<Compose<StreamSplit<F>, Zstd>>());
    encodings.emplace_back(std::make_shared<Compose<StreamSplit<F>, Lz4>>());

    std::vector<std::shared_ptr<Encoding>> shortSplits;
    shortSplits.emplace_back(std::make_shared<Compose<StreamSplit<uint16_t>, Bsc>>());
    shortSplits.emplace_back(std::make_shared<Compose<StreamSplit<uint16_t>, Zstd>>());
    shortSplits.emplace_back(std::make_shared<Compose<StreamSplit<uint16_t>, Lz4>>());

    std::vector<std::shared_ptr<Method<F>>> methods;
    for (auto &e : encodings)
        methods.emplace_back(std::make_shared<Lossless<F>>(e));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Pcodec<F>>()));


    for (auto &e : encodings)
        methods.emplace_back(std::make_shared<Lfzip<F, false>>(e));
    for (auto &e : shortSplits)
        methods.emplace_back(std::make_shared<Lfzip<F, false>>(e));
    methods.emplace_back(std::make_shared<Lfzip<F, true>>(std::make_shared<Bsc>()));
    methods.emplace_back(std::make_shared<Lfzip<F, true>>(std::make_shared<Zstd>()));
    methods.emplace_back(std::make_shared<Lfzip<F, true>>(std::make_shared<Lz4>()));

    for (auto &e : encodings)
        methods.emplace_back(std::make_shared<Quantise<F, false>>(e));
    for (auto &e : shortSplits)
        methods.emplace_back(std::make_shared<Quantise<F, false>>(e));
    methods.emplace_back(std::make_shared<Quantise<F, true>>(std::make_shared<Bsc>()));
    methods.emplace_back(std::make_shared<Quantise<F, true>>(std::make_shared<Zstd>()));
    methods.emplace_back(std::make_shared<Quantise<F, true>>(std::make_shared<Lz4>()));

    methods.emplace_back(std::make_shared<Sz3<F>>());
    return methods;
}

template <> std::vector<std::shared_ptr<Method<float>>> get_all_methods()
{
    return get_all_common_methods<float>();
}
template <> std::vector<std::shared_ptr<Method<double>>> get_all_methods()
{
    auto methods = get_all_common_methods<double>();
    methods.emplace_back(std::make_shared<Machete>());
    return methods;
}

std::vector<std::string> get_all_names()
{
    std::map<std::string, std::string> names;
    for (auto &m : get_all_methods<float>())
    {
        names.insert({m->name(), std::string("float")});
    }
    for (auto &m : get_all_methods<double>())
    {
        auto val = names.find(m->name());
        if (val == names.end())
            names.insert({m->name(), std::string("double")});
        else
            val->second = val->second + ",double";
    }
    std::vector<std::string> output;
    output.reserve(names.size());
    for (auto &n : names)
        output.push_back(n.first + ": " + n.second);
    return output;
}

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

template <typename F> std::span<const F> as_span(const std::vector<std::byte> &input)
{
    assert(input.size() % sizeof(F) == 0);
    auto data_ptr = reinterpret_cast<const F *>(input.data());
    return std::span<const F>(data_ptr, input.size() / sizeof(F));
}
template std::span<const float> as_span(const std::vector<std::byte> &input);
template std::span<const double> as_span(const std::vector<std::byte> &input);
template std::span<const int16_t> as_span(const std::vector<std::byte> &input);

template <typename F> std::span<F> as_span(std::vector<std::byte> &input)
{
    assert(input.size() % sizeof(F) == 0);
    auto data_ptr = reinterpret_cast<F *>(input.data());
    return std::span<F>(data_ptr, input.size() / sizeof(F));
}
template std::span<float> as_span(std::vector<std::byte> &input);
template std::span<double> as_span(std::vector<std::byte> &input);
template std::span<int16_t> as_span(std::vector<std::byte> &input);

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