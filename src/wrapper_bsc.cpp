#include "wrapper.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

bool bsc_initialised = false;

void init_bsc()
{
    if (!bsc_initialised)
    {
        bsc_init(bsc_init(BSC_FEATURES));
        bsc_initialised = true;
    }
}

std::vector<std::byte> Bsc::compress(std::span<const std::byte> input)
{
    init_bsc();

    std::vector<std::byte> output_buffer(LIBBSC_HEADER_SIZE + input.size_bytes());
    auto input_ptr = reinterpret_cast<const unsigned char *>(input.data());
    auto output_ptr = reinterpret_cast<unsigned char *>(output_buffer.data());
    auto compressed_sz = bsc_compress(input_ptr, output_ptr, input.size_bytes(), 0, 0, LIBBSC_BLOCKSORTER_BWT,
                                      LIBBSC_CODER_QLFC_STATIC, BSC_FEATURES);
    output_buffer.resize(compressed_sz);
    return output_buffer;
}

std::vector<std::byte> Bsc::decompress(std::span<const std::byte> input)
{
    init_bsc();

    int block_size, block_data_size;
    auto input_ptr = reinterpret_cast<const unsigned char *>(input.data());
    bsc_block_info(input_ptr, LIBBSC_HEADER_SIZE, &block_size, &block_data_size, BSC_FEATURES);
    std::vector<std::byte> decompressed_buffer(block_data_size);
    bsc_decompress(input_ptr, block_size, reinterpret_cast<unsigned char *>(decompressed_buffer.data()),
                   block_data_size, BSC_FEATURES);
    return decompressed_buffer;
}