#include <cassert>
#include <cstddef>
#include <exception>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

std::span<const float> as_float_span(const std::vector<std::byte> &input);

std::span<float> as_float_span(std::vector<std::byte> &input);

template <typename T> void vec_to_file(std::string path, const std::vector<T> &data);

template <typename T> std::vector<T> vec_from_file(std::string path);

// https://stackoverflow.com/a/26221725
template <typename... Args> std::string string_format(const std::string &format, Args... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size_s <= 0)
    {
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

inline void _do_pack_streams(std::vector<std::byte> &data)
{
}

template <typename Type, typename... Vecs>
void _do_pack_streams(std::vector<std::byte> &data, std::vector<Type> vec, Vecs... vecs)
{
    size_t vec_size = vec.size();
    auto sz = std::as_bytes(std::span<size_t, 1>(&vec_size, 1));
    auto stream = std::as_bytes(std::span(vec));
    data.insert(data.end(), sz.begin(), sz.end());
    data.insert(data.end(), stream.begin(), stream.end());
    _do_pack_streams(data, vecs...);
    return;
}

inline size_t _get_packed_length()
{
    return 0;
}

template <typename Type, typename... Vecs> size_t _get_packed_length(std::vector<Type> vec, Vecs... vecs)
{
    return sizeof(size_t) + sizeof(Type) * vec.size() + _get_packed_length(vecs...);
}

template <typename... Vecs> std::vector<std::byte> pack_streams(Vecs... vecs)
{
    auto packed_length = _get_packed_length(vecs...);
    std::vector<std::byte> data;
    data.reserve(packed_length);
    _do_pack_streams(data, vecs...);
    assert(packed_length == data.size());
    return data;
}

template <typename... Spans> void _do_unpack_streams(const std::vector<std::byte> &data, size_t index)
{
    if (index != data.size())
        throw std::runtime_error("bad unpacking");
}

template <typename Type, typename... Spans>
void _do_unpack_streams(const std::vector<std::byte> &data, size_t index, std::span<const Type> &span, Spans &...spans)
{
    size_t sz = *reinterpret_cast<const size_t *>(&data[index]);
    span = std::span<const Type>(reinterpret_cast<const Type *>(&data[index + sizeof(sz)]), sz);
    auto data_end = reinterpret_cast<const void *>(&data[data.size()]);
    auto span_end = reinterpret_cast<const void *>(&span[span.size()]);
    if (span_end > data_end)
    {
        throw std::runtime_error("bad unpacking");
    }
    _do_unpack_streams(data, index + sizeof(sz) + sizeof(Type) * sz, spans...);
}

template <typename... Spans> void unpack_streams(const std::vector<std::byte> &data, Spans &...spans)
{
    _do_unpack_streams(data, 0, spans...);
}