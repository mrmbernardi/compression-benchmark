#pragma once
#include <cstddef>
#include <cstdint>
#include <libbsc.h>
#include <span>
#include <string>
#include <vector>

#define BSC_FEATURES (LIBBSC_FEATURE_FASTMODE | LIBBSC_FEATURE_MULTITHREADING)

struct Bsc
{
    inline static const std::string name = "Bsc";
    static std::vector<std::byte> compress(std::span<const std::byte> input);
    static std::vector<std::byte> decompress(std::span<const std::byte> input);
};

struct Zstd
{
    inline static const std::string name = "Zstd";
    static std::vector<std::byte> compress(std::span<const std::byte> input);
    static std::vector<std::byte> decompress(std::span<const std::byte> input);
};