#pragma once
#include <cstddef>
#include <cstdint>
#include <libbsc.h>
#include <span>
#include <string>
#include <vector>


struct Bsc
{
    inline static const std::string name = "Bsc";
    static std::vector<std::byte> encode(std::span<const std::byte> input);
    static std::vector<std::byte> decode(std::span<const std::byte> input);
};

struct Zstd
{
    inline static const std::string name = "Zstd";
    static std::vector<std::byte> encode(std::span<const std::byte> input);
    static std::vector<std::byte> decode(std::span<const std::byte> input);
};

struct Lz4
{
    inline static const std::string name = "Lz4";
    static std::vector<std::byte> encode(std::span<const std::byte> input);
    static std::vector<std::byte> decode(std::span<const std::byte> input);
};

template <typename T> struct StreamSplit
{
    inline static const std::string name = "Stream Split";
    static std::vector<std::byte> encode(std::span<const std::byte> input);
    static std::vector<std::byte> decode(std::span<const std::byte> input);
};

template <typename W1, typename W2> struct Compose
{
    inline static const std::string name = W1::name + " with " + W2::name;
    static std::vector<std::byte> encode(std::span<const std::byte> input)
    {
        return W2::encode(W1::encode(input));
    }
    static std::vector<std::byte> decode(std::span<const std::byte> input)
    {
        return W1::decode(W2::decode(input));
    }
};