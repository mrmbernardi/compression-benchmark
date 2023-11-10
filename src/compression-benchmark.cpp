#include <string>
#include <vector>
#include <random>
#include <libbsc.h>
#include <iostream>
#include <cassert>
#define BSC_FEATURES (LIBBSC_FEATURE_FASTMODE | LIBBSC_FEATURE_MULTITHREADING)

int main(int argc, char **argv)
{
    bsc_init(BSC_FEATURES);

    std::vector<float> original_buffer(20000000);
    std::mt19937 gen;
    std::uniform_real_distribution<> dis(-500, +500);
    for (float &x : original_buffer)
    {
        x = dis(gen);
    }
    size_t original_sz = original_buffer.size() * sizeof(original_buffer[0]);
    std::vector<unsigned char> compressed_buffer(LIBBSC_HEADER_SIZE + original_sz);
    int compressed_sz = bsc_compress(reinterpret_cast<unsigned char *>(original_buffer.data()), compressed_buffer.data(), original_sz, 0, 0, LIBBSC_BLOCKSORTER_BWT, LIBBSC_CODER_QLFC_STATIC, BSC_FEATURES);

    int block_size, block_data_size;
    bsc_block_info(compressed_buffer.data(), LIBBSC_HEADER_SIZE, &block_size, &block_data_size, BSC_FEATURES);
    std::vector<float> decompressed_buffer(block_data_size / 4);
    bsc_decompress(compressed_buffer.data(), block_size, reinterpret_cast<unsigned char *>(decompressed_buffer.data()), block_data_size, BSC_FEATURES);
    for (size_t i = 0; i < original_buffer.size(); i++)
    {
        assert(original_buffer[i] == decompressed_buffer[i]);
    }
    std::cout << "Original size:   " << original_sz << std::endl;
    std::cout << "Compressed size: " << compressed_sz << std::endl;
    std::cout << "Ratio: " << (100.0f * compressed_sz / original_sz) << "%" << std::endl;
    return 0;
}