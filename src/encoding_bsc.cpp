#include "encoding.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <libbsc.h>
#include <stdexcept>
#include <string>
#include <vector>

#define BSC_FEATURES (LIBBSC_FEATURE_FASTMODE)

bool bsc_initialised = false;

void init_bsc()
{
    if (!bsc_initialised)
    {
        bsc_init(BSC_FEATURES);
        bsc_initialised = true;
    }
}

std::span<const std::byte> Bsc::encode(std::span<const std::byte> input)
{
    init_bsc();

    compressed_buffer = std::vector<std::byte>(LIBBSC_HEADER_SIZE + input.size_bytes());
    auto input_ptr = reinterpret_cast<const unsigned char *>(input.data());
    auto output_ptr = reinterpret_cast<unsigned char *>(compressed_buffer.data());
    auto compressed_sz = bsc_compress(input_ptr, output_ptr, input.size_bytes(), 0, 0, LIBBSC_DEFAULT_BLOCKSORTER,
                                      LIBBSC_CODER_QLFC_FAST, BSC_FEATURES);
    if (compressed_sz < 0)
    {
        throw std::runtime_error("bsc compression failed with " + std::to_string(compressed_sz));
    }
    compressed_buffer.resize(compressed_sz);
    return compressed_buffer;
}

std::span<const std::byte> Bsc::decode(std::span<const std::byte> input)
{
    init_bsc();

    int block_size, block_data_size;
    auto input_ptr = reinterpret_cast<const unsigned char *>(input.data());
    bsc_block_info(input_ptr, LIBBSC_HEADER_SIZE, &block_size, &block_data_size, BSC_FEATURES);
    decompressed_buffer = std::vector<std::byte>(block_data_size);
    auto err = bsc_decompress(input_ptr, block_size, reinterpret_cast<unsigned char *>(decompressed_buffer.data()),
                              block_data_size, BSC_FEATURES);
    if (err < 0)
    {
        throw std::runtime_error("bsc decompression failed with " + std::to_string(err));
    }
    return decompressed_buffer;
}