#include "benchmark.hpp"
#include "method.hpp"
#include "wrapper.hpp"
#include <random>

std::vector<float> generate_random_data(size_t size)
{
    std::vector<float> data(size);
    std::mt19937 gen;
    std::uniform_real_distribution<> dis(-500, +500);
    std::cout << "Generating random floating point data... ";
    std::cout.flush();
    for (float &x : data)
    {
        x = dis(gen);
    }
    std::cout << "done." << std::endl;
    return data;
}

std::string bench_result::to_string()
{
    std::ostringstream s;
    s << "Original size:        " << original_size << std::endl;
    s << "Compressed size:      " << compressed_size << std::endl;
    s << "Ratio:                " << ((100.0f * compressed_size) / original_size) << "%" << std::endl;
    s << "Compression time:     " << compression_time << "s" << std::endl;
    s << "Decompression time:   " << decompression_time << "s" << std::endl;
    s << "Mean Absolute Error:  " << mean_absolute_error << std::endl;
    s << "Max Error:            " << max_error << std::endl;
    return s.str();
}
