#pragma once
extern "C"
{
#include "cpcodec.h"
}
#include <cstddef>
#include <span>
#include <string>
#include <vector>

class Encoding
{
  public:
    virtual std::string name() = 0;
    virtual std::span<const std::byte> encode(std::span<const std::byte> input) = 0;
    virtual std::span<const std::byte> decode(std::span<const std::byte> input) = 0;
    virtual ~Encoding(){};
};

class Bsc : public Encoding
{
    std::vector<std::byte> compressed_buffer;
    std::vector<std::byte> decompressed_buffer;

  public:
    std::string name() override
    {
        return "Bsc";
    };
    std::span<const std::byte> encode(std::span<const std::byte> input) override;
    std::span<const std::byte> decode(std::span<const std::byte> input) override;
};

class Zstd : public Encoding
{
    std::vector<std::byte> compressed_buffer;
    std::vector<std::byte> decompressed_buffer;

  public:
    std::string name() override;
    std::span<const std::byte> encode(std::span<const std::byte> input) override;
    std::span<const std::byte> decode(std::span<const std::byte> input) override;
};

class Lz4 : public Encoding
{
    std::vector<std::byte> compressed_buffer;
    std::vector<std::byte> decompressed_buffer;

  public:
    std::string name() override
    {
        return "Lz4";
    };
    std::span<const std::byte> encode(std::span<const std::byte> input) override;
    std::span<const std::byte> decode(std::span<const std::byte> input) override;
};

class Snappy : public Encoding
{
    std::vector<std::byte> compressed_buffer;
    std::vector<std::byte> decompressed_buffer;

  public:
    std::string name() override
    {
        return "Snappy";
    };
    std::span<const std::byte> encode(std::span<const std::byte> input) override;
    std::span<const std::byte> decode(std::span<const std::byte> input) override;
};

enum PcodecEncType
{
    p_float,
    p_uint,
    p_int
};

template <typename T, PcodecEncType P> class Pcodec : public Encoding
{
    PcoFfiVec enc_vec = {};
    PcoFfiVec dec_vec = {};

  public:
    std::string name() override
    {
        if (P == p_int)
            return "Pcodec (int)";
        if (P == p_uint)
            return "Pcodec (uint)";
        if (P == p_float)
            return "Pcodec";
    };
    std::span<const std::byte> encode(std::span<const std::byte> input) override;
    std::span<const std::byte> decode(std::span<const std::byte> input) override;
    ~Pcodec();
};

template <typename T> class Gorilla : public Encoding
{
    std::vector<std::byte> compressed_buffer;
    std::vector<std::byte> decompressed_buffer;

  public:
    std::string name() override
    {
        return "Gorilla";
    };
    std::span<const std::byte> encode(std::span<const std::byte> input) override;
    std::span<const std::byte> decode(std::span<const std::byte> input) override;
};

template <typename T> std::vector<std::byte> streamsplit_enc(std::span<const std::byte> input);
template <typename T> std::vector<std::byte> streamsplit_dec(std::span<const std::byte> input);
template <typename T> class StreamSplit : public Encoding
{
    std::vector<std::byte> encoded_buffer;
    std::vector<std::byte> decoded_buffer;

  public:
    std::string name() override
    {
        return "Stream Split (" + std::to_string(sizeof(T)) + ")";
    };
    std::span<const std::byte> encode(std::span<const std::byte> input) override;
    std::span<const std::byte> decode(std::span<const std::byte> input) override;
};

template <typename E1, typename E2> class Compose : public Encoding
{
    E1 e1;
    E2 e2;

  public:
    std::string name() override
    {
        return e1.name() + " with " + e2.name();
    };
    std::span<const std::byte> encode(std::span<const std::byte> input) override
    {
        return e2.encode(e1.encode(input));
    }
    std::span<const std::byte> decode(std::span<const std::byte> input) override
    {
        return e1.decode(e2.decode(input));
    }
};