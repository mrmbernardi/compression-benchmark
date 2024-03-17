#include "encoding.hpp"
#include "gorilla.hpp"
#include <cstddef>
#include <limits>
#include <stdexcept>

template <typename F> struct types;
template <> struct types<double>
{
    typedef UInt64 uint_type;
};
template <> struct types<float>
{
    typedef UInt32 uint_type;
};

template <typename T> std::span<const std::byte> Gorilla<T>::encode(std::span<const std::byte> input)
{
    if (input.size_bytes() > std::numeric_limits<UInt32>::max())
    {
        throw std::runtime_error("Input data too big");
    }
    size_t input_sz = input.size_bytes();
    auto input_ptr = reinterpret_cast<const char *>(input.data());
    compressed_buffer = std::vector<std::byte>(getCompressedDataSize<T>(input.size_bytes()) + sizeof(size_t));
    auto output_ptr = reinterpret_cast<char *>(compressed_buffer.data());
    *reinterpret_cast<size_t *>(output_ptr) = input_sz;
    size_t output_sz = compressDataForType<typename types<T>::uint_type>(
        input_ptr, input_sz, output_ptr + sizeof(size_t), compressed_buffer.size());
    compressed_buffer.resize(output_sz + sizeof(size_t));
    return compressed_buffer;
}

template <typename T> std::span<const std::byte> Gorilla<T>::decode(std::span<const std::byte> input)
{
    size_t input_sz = input.size_bytes();
    auto input_ptr = reinterpret_cast<const char *>(input.data());
    size_t result_sz = *reinterpret_cast<const size_t *>(input_ptr);
    decompressed_buffer = std::vector<std::byte>(result_sz);
    auto output_ptr = reinterpret_cast<char *>(decompressed_buffer.data());
    decompressDataForType<typename types<T>::uint_type>(input_ptr + sizeof(size_t), input_sz - sizeof(size_t),
                                                        output_ptr, result_sz);
    return decompressed_buffer;
}
template class Gorilla<float>;
template class Gorilla<double>;