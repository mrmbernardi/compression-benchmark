#include "method.hpp"
#include <cstddef>
#include <machete_c.h>
#include <stdexcept>
#include <type_traits>

size_t Machete::compress(std::span<const double> input)
{
    if (input.size() > 65536)
    {
        throw std::runtime_error("Machete only supports up to 65536 values at a time");
    }
    free_data();
    out_sz =
        machete_compress_huffman_tabtree(const_cast<double *>(input.data()), input.size(), &compressed_buffer, error);
    in_sz = input.size();
    return out_sz;
}

std::span<const double> Machete::decompress()
{
    results = std::vector<double>(in_sz);
    machete_decompress_huffman_tabtree(compressed_buffer, out_sz, results.data(), error);
    return results;
}