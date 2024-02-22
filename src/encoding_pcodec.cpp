#include "encoding.hpp"
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

template <typename T> unsigned char get_pco_type();
template <> unsigned char get_pco_type<float>()
{
    return PCO_TYPE_F32;
}
template <> unsigned char get_pco_type<double>()
{
    return PCO_TYPE_F64;
}

template <typename T> std::span<const std::byte> Pcodec<T>::encode(std::span<const std::byte> input)
{
    if (enc_vec.raw_box)
        free_pcovec(&enc_vec);
    if (auto_compress(input.data(), input.size_bytes() / sizeof(T), get_pco_type<T>(), 8, &enc_vec) !=
        PcoError::Success)
        throw std::runtime_error("Pcodec compression failed.");
    return std::span<const std::byte>(reinterpret_cast<const std::byte *>(enc_vec.ptr), enc_vec.len);
}

template <typename T> std::span<const std::byte> Pcodec<T>::decode(std::span<const std::byte> input)
{
    if (dec_vec.raw_box)
        free_pcovec(&dec_vec);
    if (auto_decompress(input.data(), input.size_bytes(), get_pco_type<T>(), &dec_vec) != PcoError::Success)
        throw std::runtime_error("Pcodec decompression failed.");
    return std::span<const std::byte>(reinterpret_cast<const std::byte *>(dec_vec.ptr), dec_vec.len * sizeof(T));
}

template <typename T> Pcodec<T>::~Pcodec()
{
    if (dec_vec.raw_box)
        free_pcovec(&dec_vec);
    if (enc_vec.raw_box)
        free_pcovec(&enc_vec);
}
template class Pcodec<float>;
template class Pcodec<double>;