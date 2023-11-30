#pragma once
#include "encoding.hpp"
#include <cstddef>
#include <cstdint>
#include <future>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <vector>

class Method
{
  public:
    virtual std::string name() = 0;
    virtual size_t compress(std::span<const float> input) = 0;
    virtual std::span<const float> decompress() = 0;
    virtual ~Method(){};
};

class Lossless : public Method
{
    std::vector<std::byte> compressed_buffer;
    std::vector<std::byte> decompressed_buffer;
    std::shared_ptr<Encoding> encoding;

  public:
    Lossless(std::shared_ptr<Encoding> e) : encoding(e){};
    std::string name() override
    {
        return encoding->name() + " (lossless)";
    };
    size_t compress(const std::span<const float> input) override;
    std::span<const float> decompress() override;
};

class Sz3 : public Method
{
    std::unique_ptr<char[]> compressed_data;
    size_t compressed_size;
    std::unique_ptr<float[]> decompressed_data;
    size_t decompressed_size;

  public:
    std::string name() override
    {
        return "Sz3";
    };
    size_t compress(std::span<const float> input) override;
    std::span<const float> decompress() override;
};

class Lfzip : public Method
{
    float error = 1.0f - 1e-06f; // this is nonsense which i'm only replicating to match the lfzip code.
    float maxerror_original = 1.0f;
    static constexpr size_t filter_size = 32;
    std::vector<std::byte> compressed_buffer;
    std::vector<float> result;
    std::shared_ptr<Encoding> encoding;

  public:
    Lfzip(std::shared_ptr<Encoding> e) : encoding(e){};
    std::string name() override
    {
        return "LfZip with " + encoding->name();
    };
    size_t compress(std::span<const float> input) override;
    std::span<const float> decompress() override;
};

class Quantise : public Method
{
    static constexpr float error = 1.0f;
    std::vector<std::byte> compressed_buffer;
    std::vector<float> result;
    std::shared_ptr<Encoding> encoding;

  public:
    Quantise(std::shared_ptr<Encoding> e) : encoding(e){};
    std::string name() override
    {
        return "Quantise with " + encoding->name();
    };
    size_t compress(std::span<const float> input) override;
    std::span<const float> decompress() override;
};

class Machete : public Method
{
    uint8_t* compressed_buffer = nullptr;

  public:
    std::string name() override
    {
        return "Machete";
    };
    size_t compress(std::span<const float> input) override;
    std::span<const float> decompress() override;
};