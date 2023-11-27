#pragma once
#include <cstddef>
#include <cstdint>
#include <libbsc.h>
#include <span>
#include <string>
#include <vector>

struct Wrapper
{
    virtual std::string name() = 0;
    virtual std::vector<std::byte> encode(std::span<const std::byte> input) = 0;
    virtual std::vector<std::byte> decode(std::span<const std::byte> input) = 0;
};

struct Bsc : Wrapper
{
    std::string name() override
    {
        return "Bsc";
    };
    std::vector<std::byte> encode(std::span<const std::byte> input) override;
    std::vector<std::byte> decode(std::span<const std::byte> input) override;
};

struct Zstd : Wrapper
{
    std::string name() override
    {
        return "Zstd";
    }
    std::vector<std::byte> encode(std::span<const std::byte> input) override;
    std::vector<std::byte> decode(std::span<const std::byte> input) override;
};

struct Lz4 : Wrapper
{
    std::string name() override
    {
        return "Lz4";
    };
    std::vector<std::byte> encode(std::span<const std::byte> input) override;
    std::vector<std::byte> decode(std::span<const std::byte> input) override;
};

template <typename T> struct StreamSplit : Wrapper
{
    std::string name() override
    {
        return "Stream Split";
    };
    std::vector<std::byte> encode(std::span<const std::byte> input) override;
    std::vector<std::byte> decode(std::span<const std::byte> input) override;
};

template <typename W1, typename W2> struct Compose : Wrapper
{
    W1 w1;
    W2 w2;
    std::string name() override
    {
        return w1.name() + " with " + w2.name();
    };
    std::vector<std::byte> encode(std::span<const std::byte> input) override
    {
        return w2.encode(w1.encode(input));
    }
    std::vector<std::byte> decode(std::span<const std::byte> input) override
    {
        return w1.decode(w2.decode(input));
    }
};