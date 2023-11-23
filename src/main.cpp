#include "benchmark.hpp"
#include "method.hpp"
#include "tabulate/font_align.hpp"
#include "tabulate/table.hpp"
#include "util.hpp"
#include "wrapper.hpp"
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

using namespace tabulate;

int main(/* int argc, char **argv */)
{
    std::vector<float> original_buffer = generate_random_data(2000000); // 20000000);
    // for (float &v : original_buffer)
    //     v *= 500;
    vec_to_file("input.vec", original_buffer);

    std::vector<bench_result> results;
    Table table;
    results.emplace_back(benchmark<Lossless<Bsc>>(original_buffer));
    results.emplace_back(benchmark<Lossless<Zstd>>(original_buffer));
    results.emplace_back(benchmark<Lfzip<Bsc>>(original_buffer));
    results.emplace_back(benchmark<Lfzip<Zstd>>(original_buffer));
    results.emplace_back(benchmark<Sz3>(original_buffer));

    table.add_row({"Method", "Ratio", "Compression Time (s)", "Deompression Time (s)", "Max Error", "MAE"});
    for (bench_result r : results)
    {
        table.add_row({r.name, string_format("%.2f%%", (r.compressed_size * 100.f / r.original_size)),
                       string_format("%f", r.compression_time), string_format("%f", r.decompression_time),
                       string_format("%f", r.max_error), string_format("%f", r.mean_absolute_error)});
    }

    for (size_t col = 1; col < table.row(0).size(); col++)
        for (size_t row = 1; row < table.size(); row++)
            table[row][col].format().font_align(FontAlign::right);

    std::cout << table << std::endl;
    return 0;
}