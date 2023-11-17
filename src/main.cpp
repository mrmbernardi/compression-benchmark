#include "benchmark.hpp"
#include "method.hpp"
#include <cstdint>

int main(/* int argc, char **argv */)
{
    std::vector<float> original_buffer = generate_random_data(2000); // 20000000);
    benchmark<Lfzip>(original_buffer);
    return 0;
}