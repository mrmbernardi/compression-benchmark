#include "benchmark.hpp"
#include "method.hpp"
#include "wrapper.hpp"
#include "util.hpp"
#include <cstdint>

int main(/* int argc, char **argv */)
{
    std::vector<float> original_buffer = generate_random_data(2000000); // 20000000);
    // for (float &v : original_buffer)
    //     v *= 500;
    vec_to_file("input.vec", original_buffer);
    benchmark<Lossless<Bsc>>(original_buffer);
    benchmark<Lossless<Zstd>>(original_buffer);
    benchmark<Lfzip<Bsc>>(original_buffer);
    benchmark<Lfzip<Zstd>>(original_buffer);
    benchmark<Sz3>(original_buffer);
    return 0;
}