#include "byte_stream_split_internal.hpp"
#include "encoding.hpp"
#include <cassert>
#include <cstddef>
#include <vector>

using namespace arrow::util::internal;

template <typename T> std::vector<std::byte> StreamSplit<T>::encode(std::span<const std::byte> input)
{
    assert(input.size_bytes() % sizeof(T) == 0);
    size_t num_vals = input.size_bytes() / sizeof(T);

    std::vector<std::byte> output_buffer = std::vector<std::byte>(input.size_bytes());
    ByteStreamSplitEncodeAvx2<float>(reinterpret_cast<const uint8_t *>(input.data()), num_vals,
                                     reinterpret_cast<uint8_t *>(output_buffer.data()));
    return output_buffer;
}

template <typename T> std::vector<std::byte> StreamSplit<T>::decode(std::span<const std::byte> input)
{
    assert(input.size_bytes() % sizeof(T) == 0);
    size_t num_vals = input.size_bytes() / sizeof(T);

    std::vector<std::byte> output_buffer = std::vector<std::byte>(input.size_bytes());
    ByteStreamSplitDecodeAvx2<T>(reinterpret_cast<const uint8_t *>(input.data()), num_vals, num_vals,
                                 reinterpret_cast<T *>(output_buffer.data()));
    return output_buffer;
}
template struct StreamSplit<float>;
template struct StreamSplit<double>;