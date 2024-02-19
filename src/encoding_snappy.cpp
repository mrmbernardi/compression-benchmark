#include "encoding.hpp"
#include <cassert>
#include <cstddef>
#include <snappy.h>
#include <stdexcept>

std::vector<std::byte> Snappy::encode(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    auto input_ptr = reinterpret_cast<const char *>(input.data());
    std::vector<std::byte> output_buffer(snappy::MaxCompressedLength(input_sz));
    auto output_ptr = reinterpret_cast<char *>(output_buffer.data());
    size_t output_sz;
    snappy::RawCompress(input_ptr, input_sz, output_ptr, &output_sz);
    output_buffer.resize(output_sz);
    return output_buffer;
}

std::vector<std::byte> Snappy::decode(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    auto input_ptr = reinterpret_cast<const char *>(input.data());
    size_t result;
    if (!snappy::GetUncompressedLength(input_ptr, input_sz, &result))
        throw std::runtime_error("Snappy decompression failed.");
    std::vector<std::byte> output_buffer(result);
    auto output_ptr = reinterpret_cast<char *>(output_buffer.data());
    snappy::RawUncompress(input_ptr, input_sz, output_ptr);
    return output_buffer;
}