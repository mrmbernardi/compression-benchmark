#include "benchmark.hpp"
#include "method.hpp"
#include "tabulate/font_align.hpp"
#include "tabulate/table.hpp"
#include "util.hpp"
#include "encoding.hpp"
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

using namespace tabulate;

int main(/* int argc, char **argv */)
{
    std::vector<float> original_buffer = generate_random_data(200000); // 20000000);
    // for (float &v : original_buffer)
    //     v *= 50000;

    std::vector<bench_result> results;
    results.emplace_back(benchmark<Lossless>(original_buffer, Bsc()));
    results.emplace_back(benchmark<Lossless>(original_buffer, Zstd()));
    results.emplace_back(benchmark<Lossless>(original_buffer, Lz4()));

    results.emplace_back(benchmark<Lossless>(original_buffer, Compose<StreamSplit<float>, Bsc>()));
    results.emplace_back(benchmark<Lossless>(original_buffer, Compose<StreamSplit<float>, Zstd>()));
    results.emplace_back(benchmark<Lossless>(original_buffer, Compose<StreamSplit<float>, Lz4>()));

    results.emplace_back(benchmark<Lfzip>(original_buffer, Bsc()));
    results.emplace_back(benchmark<Lfzip>(original_buffer, Zstd()));
    results.emplace_back(benchmark<Lfzip>(original_buffer, Lz4()));

    results.emplace_back(benchmark<Lfzip>(original_buffer, Compose<StreamSplit<float>, Bsc>()));
    results.emplace_back(benchmark<Lfzip>(original_buffer, Compose<StreamSplit<float>, Zstd>()));
    results.emplace_back(benchmark<Lfzip>(original_buffer, Compose<StreamSplit<float>, Lz4>()));

    results.emplace_back(benchmark<Quantise>(original_buffer, Bsc()));
    results.emplace_back(benchmark<Quantise>(original_buffer, Zstd()));
    results.emplace_back(benchmark<Quantise>(original_buffer, Lz4()));

    results.emplace_back(benchmark<Quantise>(original_buffer, Compose<StreamSplit<float>, Bsc>()));
    results.emplace_back(benchmark<Quantise>(original_buffer, Compose<StreamSplit<float>, Zstd>()));
    results.emplace_back(benchmark<Quantise>(original_buffer, Compose<StreamSplit<float>, Lz4>()));

    results.emplace_back(benchmark<Sz3>(original_buffer));

    Table table;
    table.add_row({"Method", "Ratio", "Compression Time (s)", "Rate (MB/s)", "Decompression Time (s)", "Rate (MB/s)",
                   "Max Error", "MAE"});
    for (bench_result r : results)
    {
        table.add_row({r.name, string_format("%.2f%%", (r.compressed_size * 100.f / r.original_size)),
                       string_format("%f", r.compression_time), string_format("%f", r.compression_data_rate()),
                       string_format("%f", r.decompression_time), string_format("%f", r.decompression_data_rate()),
                       string_format("%f", r.max_error), string_format("%f", r.mean_absolute_error)});
    }

    for (size_t col = 0; col < table.row(0).size(); col++)
        table[0][col].format().font_align(FontAlign::center);
    for (size_t col = 1; col < table.row(0).size(); col++)
        for (size_t row = 1; row < table.size(); row++)
            table[row][col].format().font_align(FontAlign::right);

    std::cout << table << std::endl;
    return 0;
}