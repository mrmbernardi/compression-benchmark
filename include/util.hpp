#include "tabulate/table.hpp"
#include <cassert>
#include <cstddef>
#include <exception>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

template <typename F> class Method;

template <typename F> std::vector<std::shared_ptr<Method<F>>> get_all_methods();

std::vector<std::string> get_all_names();

void table_to_file(std::string path, tabulate::Table &table);

template <typename T> std::span<const T> as_typed_span(const std::vector<std::byte> &vec)
{
    assert(vec.size() % sizeof(T) == 0);
    auto data = reinterpret_cast<const T *>(vec.data());
    return std::span<const T>(data, vec.size() / sizeof(T));
}

template <typename T> inline std::span<const T> as_typed_span(std::span<const std::byte> span)
{
    assert(span.size_bytes() % sizeof(T) == 0);
    auto data = reinterpret_cast<const T *>(span.data());
    return std::span<const T>(data, span.size_bytes() / sizeof(T));
}

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

template <typename Type, typename... Spans>
void _do_pack_streams(std::vector<std::byte> &data, std::span<Type> span, Spans... spans)
{
    size_t span_size = span.size();
    auto sz = std::as_bytes(std::span<size_t, 1>(&span_size, 1));
    auto stream = std::as_bytes(std::span(span));
    data.insert(data.end(), sz.begin(), sz.end());
    data.insert(data.end(), stream.begin(), stream.end());
    _do_pack_streams(data, spans...);
    return;
}

inline size_t _get_packed_length()
{
    return 0;
}

template <typename Type, typename... Spans> size_t _get_packed_length(std::span<Type> span, Spans... spans)
{
    return sizeof(size_t) + sizeof(Type) * span.size() + _get_packed_length(spans...);
}

template <typename... Spans> std::vector<std::byte> pack_streams(Spans... spans)
{
    auto packed_length = _get_packed_length(spans...);
    std::vector<std::byte> data;
    data.reserve(packed_length);
    _do_pack_streams(data, spans...);
    assert(packed_length == data.size());
    return data;
}

template <typename... Spans> void _do_unpack_streams(std::span<const std::byte> &data, size_t index)
{
    if (index != data.size())
        throw std::runtime_error("bad unpacking");
}

template <typename Type, typename... Spans>
void _do_unpack_streams(std::span<const std::byte> &data, size_t index, std::span<const Type> &span, Spans &...spans)
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

template <typename... Spans> void unpack_streams(std::span<const std::byte> &data, Spans &...spans)
{
    _do_unpack_streams(data, 0, spans...);
}