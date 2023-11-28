#pragma once
#include <cstddef>
#include <cstdint>
#include <libbsc.h>
#include <span>
#include <string>
#include <vector>

struct Encoding
{
    virtual std::string name() = 0;
    virtual std::vector<std::byte> encode(std::span<const std::byte> input) = 0;
    virtual std::vector<std::byte> decode(std::span<const std::byte> input) = 0;
    virtual ~Encoding(){};
};

struct Bsc : Encoding
{
    std::string name() override
    {
        return "Bsc";
    };
    std::vector<std::byte> encode(std::span<const std::byte> input) override;
    std::vector<std::byte> decode(std::span<const std::byte> input) override;
};

struct Zstd : Encoding
{
    std::string name() override
    {
        return "Zstd";
    }
    std::vector<std::byte> encode(std::span<const std::byte> input) override;
    std::vector<std::byte> decode(std::span<const std::byte> input) override;
};

struct Lz4 : Encoding
{
    std::string name() override
    {
        return "Lz4";
    };
    std::vector<std::byte> encode(std::span<const std::byte> input) override;
    std::vector<std::byte> decode(std::span<const std::byte> input) override;
};

template <typename T> struct StreamSplit : Encoding
{
    std::string name() override
    {
        return "Stream Split";
    };
    std::vector<std::byte> encode(std::span<const std::byte> input) override;
    std::vector<std::byte> decode(std::span<const std::byte> input) override;
};

template <typename E1, typename E2> struct Compose : Encoding
{
    E1 e1;
    E2 e2;
    std::string name() override
    {
        return e1.name() + " with " + e2.name();
    };
    std::vector<std::byte> encode(std::span<const std::byte> input) override
    {
        return e2.encode(e1.encode(input));
    }
    std::vector<std::byte> decode(std::span<const std::byte> input) override
    {
        return e1.decode(e2.decode(input));
    }
};