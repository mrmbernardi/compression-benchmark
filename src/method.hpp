#pragma once
#include "wrapper.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

class Lossless
{
    std::vector<std::byte> compressed_buffer;
    std::vector<std::byte> decompressed_buffer;
    Wrapper &wrapper;

  public:
    const std::string name;
    Lossless(Wrapper &w) : wrapper(w), name(w.name() + " (lossless)"){};
    size_t compress(const std::vector<float> &input);
    std::span<const float> decompress();
};

class Sz3
{
    std::unique_ptr<char[]> compressed_data;
    size_t compressed_size;
    std::unique_ptr<float[]> decompressed_data;
    size_t decompressed_size;

  public:
    inline static const std::string name = "Sz3";
    size_t compress(const std::vector<float> &input);
    std::span<const float> decompress();
};

class Lfzip
{
    float error = 1.0f - 1e-06f; // this is nonsense which i'm only replicating to match the lfzip code.
    float maxerror_original = 1.0f;
    static constexpr size_t filter_size = 32;
    std::vector<std::byte> compressed_buffer;
    Wrapper &wrapper;

  public:
    const std::string name;
    Lfzip(Wrapper &w) : wrapper(w), name("LfZip with " + w.name()){};
    size_t compress(const std::vector<float> &input);
    std::vector<float> decompress();
};

class Quantise
{
    static constexpr float error = 1.0f;
    std::vector<std::byte> compressed_buffer;
    Wrapper &wrapper;

  public:
    const std::string name;
    Quantise(Wrapper &w) : wrapper(w), name("Quantise with " + w.name()){};
    size_t compress(const std::vector<float> &input);
    std::vector<float> decompress();
};