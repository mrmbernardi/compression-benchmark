#include "util.hpp"
#include "encoding.hpp"
#include "method.hpp"
#include <cstdio>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

template <typename F> std::vector<std::shared_ptr<Method<F>>> get_all_common_methods()
{
    // return {std::make_shared<Lfzip<F, true>>(std::make_shared<Lz4>())}; // for debugging a single method

    std::vector<std::shared_ptr<Encoding>> encodings;
    encodings.emplace_back(std::make_shared<Bsc>());
    encodings.emplace_back(std::make_shared<Zstd>());
    encodings.emplace_back(std::make_shared<Lz4>());
    encodings.emplace_back(std::make_shared<Snappy>());

    std::vector<std::shared_ptr<Method<F>>> methods;
    for (auto &e : encodings)
        methods.emplace_back(std::make_shared<Lossless<F>>(e));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Pcodec<F, p_float>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Pcodec<F, p_int>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Pcodec<F, p_uint>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Gorilla<F>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Compose<StreamSplit<F>, Bsc>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Compose<StreamSplit<F>, Zstd>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Compose<StreamSplit<F>, Lz4>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Compose<StreamSplit<F>, Snappy>>()));

    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Compose<Gorilla<F>, Bsc>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Compose<Gorilla<F>, Zstd>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Compose<Gorilla<F>, Lz4>>()));
    methods.emplace_back(std::make_shared<Lossless<F>>(std::make_shared<Compose<Gorilla<F>, Snappy>>()));

    for (auto &e : encodings)
    {
        methods.emplace_back(std::make_shared<Lfzip<F, false>>(e));
        methods.emplace_back(std::make_shared<Lfzip<F, true>>(e));
    }

    for (auto &e : encodings)
    {
        methods.emplace_back(std::make_shared<Quantise<F, false>>(e));
        methods.emplace_back(std::make_shared<Quantise<F, true>>(e));
    }

    for (auto &e : encodings)
    {
        methods.emplace_back(std::make_shared<Mask<F>>(e));
    }
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Pcodec<F, p_float>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Pcodec<F, p_int>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Pcodec<F, p_uint>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Gorilla<F>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Compose<StreamSplit<F>, Bsc>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Compose<StreamSplit<F>, Zstd>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Compose<StreamSplit<F>, Lz4>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Compose<StreamSplit<F>, Snappy>>()));

    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Compose<Gorilla<F>, Bsc>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Compose<Gorilla<F>, Zstd>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Compose<Gorilla<F>, Lz4>>()));
    methods.emplace_back(std::make_shared<Mask<F>>(std::make_shared<Compose<Gorilla<F>, Snappy>>()));

    for (auto &e : encodings)
    {
        methods.emplace_back(std::make_shared<IntFloat<F>>(e));
    }
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Pcodec<F, p_float>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Pcodec<F, p_int>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Pcodec<F, p_uint>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Gorilla<F>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Compose<StreamSplit<F>, Bsc>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Compose<StreamSplit<F>, Zstd>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Compose<StreamSplit<F>, Lz4>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Compose<StreamSplit<F>, Snappy>>()));

    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Compose<Gorilla<F>, Bsc>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Compose<Gorilla<F>, Zstd>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Compose<Gorilla<F>, Lz4>>()));
    methods.emplace_back(std::make_shared<IntFloat<F>>(std::make_shared<Compose<Gorilla<F>, Snappy>>()));

    methods.emplace_back(std::make_shared<Sz3<F>>());
    methods.emplace_back(std::make_shared<Zfp<F>>());
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