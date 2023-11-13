#include <stddef.h>
#include <vector>
#include <libbsc.h>
#include "method.hpp"
#define BSC_FEATURES (LIBBSC_FEATURE_FASTMODE | LIBBSC_FEATURE_MULTITHREADING)

static bool bsc_initialised = false;

Bsc::Bsc()
{
    if (!bsc_initialised)
    {
        bsc_init(bsc_init(BSC_FEATURES));
        bsc_initialised = true;
    }
}

size_t Bsc::compress(const std::vector<float> &input)
{
    size_t input_sz = input.size() * sizeof(input[0]);
    compressed_buffer.resize(LIBBSC_HEADER_SIZE + input_sz);
    int compressed_sz = bsc_compress(reinterpret_cast<const unsigned char *>(input.data()), compressed_buffer.data(), input_sz, 0, 0, LIBBSC_BLOCKSORTER_BWT, LIBBSC_CODER_QLFC_STATIC, BSC_FEATURES);
    compressed_buffer.resize(compressed_sz);
    return compressed_buffer.size();
}

std::vector<float> Bsc::decompress()
{
    int block_size, block_data_size;
    bsc_block_info(compressed_buffer.data(), LIBBSC_HEADER_SIZE, &block_size, &block_data_size, BSC_FEATURES);
    std::vector<float> decompressed_buffer(block_data_size / 4);
    bsc_decompress(compressed_buffer.data(), block_size, reinterpret_cast<unsigned char *>(decompressed_buffer.data()), block_data_size, BSC_FEATURES);
    return std::move(decompressed_buffer);
}
