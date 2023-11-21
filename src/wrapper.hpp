#pragma once
#include <cstddef>
#include <cstdint>
#include <libbsc.h>
#include <span>
#include <vector>

#define BSC_FEATURES (LIBBSC_FEATURE_FASTMODE | LIBBSC_FEATURE_MULTITHREADING)

static bool bsc_initialised = false;

struct BscWrapper
{
    static std::vector<std::byte> compress(std::span<const std::byte> input);
    static std::vector<std::byte> decompress(std::span<const std::byte> input);
};

struct ZstdWrapper
{
    static std::vector<std::byte> compress(std::span<const std::byte> input);
    static std::vector<std::byte> decompress(std::span<const std::byte> input);
};