#include <cstddef>
#include <exception>
#include <memory>
#include <span>
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