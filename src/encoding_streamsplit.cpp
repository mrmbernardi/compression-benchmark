#include "byte_stream_split_internal.hpp"
#include "encoding.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace arrow::util::internal;

template <typename T> std::vector<std::byte> streamsplit_enc(std::span<const std::byte> input)
{
    assert(input.size_bytes() % sizeof(T) == 0);
    size_t num_vals = input.size_bytes() / sizeof(T);

    std::vector<std::byte> output_buffer = std::vector<std::byte>(input.size_bytes());
    ByteStreamSplitEncodeAvx2<T>(reinterpret_cast<const uint8_t *>(input.data()), num_vals,
                                 reinterpret_cast<uint8_t *>(output_buffer.data()));
    return output_buffer;
}
template std::vector<std::byte> streamsplit_enc<double>(std::span<const std::byte> input);
template std::vector<std::byte> streamsplit_enc<float>(std::span<const std::byte> input);
template <typename T> std::vector<std::byte> streamsplit_dec(std::span<const std::byte> input)
{
    assert(input.size_bytes() % sizeof(T) == 0);
    size_t num_vals = input.size_bytes() / sizeof(T);

    std::vector<std::byte> output_buffer = std::vector<std::byte>(input.size_bytes());
    ByteStreamSplitDecodeAvx2<T>(reinterpret_cast<const uint8_t *>(input.data()), num_vals, num_vals,
                                 reinterpret_cast<T *>(output_buffer.data()));
    return output_buffer;
}
template std::vector<std::byte> streamsplit_dec<double>(std::span<const std::byte> input);
template std::vector<std::byte> streamsplit_dec<float>(std::span<const std::byte> input);

template <> std::vector<std::byte> streamsplit_enc<uint16_t>(std::span<const std::byte> input)
{
    assert(input.size_bytes() % sizeof(uint16_t) == 0);
    size_t num_vals = input.size_bytes() / sizeof(uint16_t);

    std::vector<std::byte> output_buffer = std::vector<std::byte>(input.size_bytes());
    ByteStreamSplitEncodeScalar<uint16_t>(reinterpret_cast<const uint8_t *>(input.data()), num_vals,
                                          reinterpret_cast<uint8_t *>(output_buffer.data()));
    return output_buffer;
}
template <> std::vector<std::byte> streamsplit_dec<uint16_t>(std::span<const std::byte> input)
{
    assert(input.size_bytes() % sizeof(uint16_t) == 0);
    size_t num_vals = input.size_bytes() / sizeof(uint16_t);

    std::vector<std::byte> output_buffer = std::vector<std::byte>(input.size_bytes());
    ByteStreamSplitDecodeScalar<uint16_t>(reinterpret_cast<const uint8_t *>(input.data()), num_vals, num_vals,
                                          reinterpret_cast<uint16_t *>(output_buffer.data()));
    return output_buffer;
}

template <typename T> std::span<const std::byte> StreamSplit<T>::encode(std::span<const std::byte> input)
{
    encoded_buffer = streamsplit_enc<T>(input);
    return encoded_buffer;
}

template <typename T> std::span<const std::byte> StreamSplit<T>::decode(std::span<const std::byte> input)
{
    decoded_buffer = streamsplit_dec<T>(input);
    return decoded_buffer;
}
template class StreamSplit<float>;
template class StreamSplit<double>;
template class StreamSplit<uint16_t>;