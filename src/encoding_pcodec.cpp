#include "encoding.hpp"
#include <cstddef>
#include <span>
#include <stdexcept>

template <typename T, PcodecEncType P> unsigned char get_pco_type();
template <> unsigned char get_pco_type<float, p_float>()
{
    return PCO_TYPE_F32;
}
template <> unsigned char get_pco_type<float, p_int>()
{
    return PCO_TYPE_I32;
}
template <> unsigned char get_pco_type<float, p_uint>()
{
    return PCO_TYPE_U32;
}
template <> unsigned char get_pco_type<double, p_float>()
{
    return PCO_TYPE_F64;
}
template <> unsigned char get_pco_type<double, p_int>()
{
    return PCO_TYPE_I64;
}
template <> unsigned char get_pco_type<double, p_uint>()
{
    return PCO_TYPE_U64;
}

template <typename T, PcodecEncType P> std::span<const std::byte> Pcodec<T, P>::encode(std::span<const std::byte> input)
{
    if (enc_vec.raw_box)
        free_pcovec(&enc_vec);
    if (auto_compress(input.data(), input.size_bytes() / sizeof(T), get_pco_type<T, P>(), 8, &enc_vec) !=
        PcoError::Success)
        throw std::runtime_error("Pcodec compression failed.");
    return std::span<const std::byte>(reinterpret_cast<const std::byte *>(enc_vec.ptr), enc_vec.len);
}

template <typename T, PcodecEncType P> std::span<const std::byte> Pcodec<T, P>::decode(std::span<const std::byte> input)
{
    if (dec_vec.raw_box)
        free_pcovec(&dec_vec);
    if (auto_decompress(input.data(), input.size_bytes(), get_pco_type<T, P>(), &dec_vec) != PcoError::Success)
        throw std::runtime_error("Pcodec decompression failed.");
    return std::span<const std::byte>(reinterpret_cast<const std::byte *>(dec_vec.ptr), dec_vec.len * sizeof(T));
}

template <typename T, PcodecEncType P> Pcodec<T, P>::~Pcodec()
{
    if (dec_vec.raw_box)
        free_pcovec(&dec_vec);
    if (enc_vec.raw_box)
        free_pcovec(&enc_vec);
}
template class Pcodec<float, p_float>;
template class Pcodec<float, p_int>;
template class Pcodec<float, p_uint>;
template class Pcodec<double, p_float>;
template class Pcodec<double, p_int>;
template class Pcodec<double, p_uint>;