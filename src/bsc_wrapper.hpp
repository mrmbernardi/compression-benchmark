#pragma once
#include <cstddef>
#include <cstdint>
#include <libbsc.h>
#include <span>
#include <vector>

#define BSC_FEATURES (LIBBSC_FEATURE_FASTMODE | LIBBSC_FEATURE_MULTITHREADING)

static bool bsc_initialised = false;

std::vector<std::byte> bsc_compress_wrapper(std::span<const std::byte> input);

std::vector<std::byte> bsc_decompress_wrapper(std::span<const std::byte> input);