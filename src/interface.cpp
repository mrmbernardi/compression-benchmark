#include "benchmark.hpp"
#include "method.hpp"
#include <iostream>

extern "C" int recompress(bench_result *results, char dtype, void *data, int size)
{
    try
    {
        auto span = std::span<double>((double *)data, size);
        // auto method = Machete();
        auto method = Sz3<double>();
        auto results_ex = benchmark<double>(span, method);
        (*results).original_size = results_ex.original_size;
        (*results).compressed_size = results_ex.compressed_size;
        (*results).compression_time = results_ex.compression_time;
        (*results).decompression_time = results_ex.decompression_time;
        (*results).max_error = results_ex.max_error;
        (*results).mean_absolute_error = results_ex.mean_absolute_error;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}