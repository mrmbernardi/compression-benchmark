#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class Bsc
{
    std::vector<unsigned char> compressed_buffer;
  public:
    std::string name = "Bsc";
    Bsc();
    size_t compress(const std::vector<float> &input);
    std::vector<float> decompress();
};

class Sz3
{
    char *compressedData = nullptr;
    size_t compressedSize;
    void clear_compressed_data();

  public:
    std::string name = "Sz3";
    ~Sz3();
    size_t compress(const std::vector<float> &input);
    std::vector<float> decompress();
};

class Lfzip
{
    float error = 1.0f - 1e-06f; // this is nonsense which i'm only replicating to match the lfzip code.
    std::vector<int16_t> indices;
    static const size_t filter_size = 32;

  public:
    std::string name = "LfZip";
    size_t compress(const std::vector<float> &input);
    std::vector<float> decompress();
};