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

template <typename F> class Method
{
  public:
    virtual std::string name() = 0;
    virtual size_t compress(std::span<const F> input) = 0;
    virtual std::span<const F> decompress() = 0;
    virtual ~Method(){};
};

class Lossless : public Method<float>
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

template <typename F> class Sz3 : public Method<F>
{
    std::unique_ptr<char[]> compressed_data;
    size_t compressed_size;
    std::unique_ptr<F[]> decompressed_data;
    size_t decompressed_size;

  public:
    std::string name() override
    {
        return "Sz3";
    };
    size_t compress(std::span<const F> input) override;
    std::span<const F> decompress() override;
};

class Lfzip : public Method<float>
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

class Quantise : public Method<float>
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

class Machete : public Method<double>
{
    size_t in_sz = 0; // sort of cheating compared to the other methods.
    size_t out_sz = 0;
    uint8_t *compressed_buffer = nullptr;
    std::vector<double> results;

    void free_data()
    {
        if (compressed_buffer)
        {
            std::free(compressed_buffer);
            compressed_buffer = nullptr;
            out_sz = 0;
            in_sz = 0;
        }
    }

  public:
    std::string name() override
    {
        return "Machete";
    };
    size_t compress(std::span<const double> input) override;
    std::span<const double> decompress() override;
    ~Machete()
    {
        free_data();
    }
};