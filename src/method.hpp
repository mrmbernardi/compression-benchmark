#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

class Bsc
{
    std::vector<std::byte> compressed_buffer;
    std::vector<std::byte> decompressed_buffer;

  public:
    std::string name = "Bsc";
    size_t compress(const std::vector<float> &input);
    std::span<const float> decompress();
};

class Sz3
{
    char *compressed_data = nullptr;
    size_t compressed_size;
    float *decompressed_data = nullptr;
    size_t decompressed_size;
    void clear_data();

  public:
    std::string name = "Sz3";
    ~Sz3();
    size_t compress(const std::vector<float> &input);
    std::span<const float> decompress();
};

class Lfzip
{
    float error = 1.0f - 1e-06f; // this is nonsense which i'm only replicating to match the lfzip code.
    float maxerror_original = 1.0f;
    static const size_t filter_size = 32;
    std::vector<std::byte> compressed_buffer;

  public:
    std::string name = "LfZip";
    size_t compress(const std::vector<float> &input);
    std::vector<float> decompress();
};