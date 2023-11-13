#pragma once
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