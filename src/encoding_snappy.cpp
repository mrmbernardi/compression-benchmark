#include "encoding.hpp"
#include <cassert>
#include <cstddef>
#include <snappy.h>
#include <stdexcept>

std::span<const std::byte> Snappy::encode(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    auto input_ptr = reinterpret_cast<const char *>(input.data());
    compressed_buffer = std::vector<std::byte>(snappy::MaxCompressedLength(input_sz));
    auto output_ptr = reinterpret_cast<char *>(compressed_buffer.data());
    size_t output_sz;
    snappy::RawCompress(input_ptr, input_sz, output_ptr, &output_sz);
    compressed_buffer.resize(output_sz);
    return compressed_buffer;
}

std::span<const std::byte> Snappy::decode(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    auto input_ptr = reinterpret_cast<const char *>(input.data());
    size_t result;
    if (!snappy::GetUncompressedLength(input_ptr, input_sz, &result))
        throw std::runtime_error("Snappy decompression failed.");
    decompressed_buffer = std::vector<std::byte>(result);
    auto output_ptr = reinterpret_cast<char *>(decompressed_buffer.data());
    snappy::RawUncompress(input_ptr, input_sz, output_ptr);
    return decompressed_buffer;
}