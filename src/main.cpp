#include "benchmark.hpp"
#include "encoding.hpp"
#include "method.hpp"
#include "tabulate/font_align.hpp"
#include "tabulate/table.hpp"
#include "util.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>
#include <vector>

using namespace tabulate;

typedef double real;

int main(/* int argc, char **argv */)
{
    std::vector<real> original_buffer = generate_random_data<real>(65536); // 20000000);
    // for (real &v : original_buffer)
    //     v *= 50000;
    // vec_to_file("data.vec", original_buffer);

    std::vector<std::shared_ptr<Encoding>> encodings;
    encodings.emplace_back(std::make_shared<Bsc>());
    encodings.emplace_back(std::make_shared<Zstd>());
    encodings.emplace_back(std::make_shared<Lz4>());
    encodings.emplace_back(std::make_shared<Compose<StreamSplit<real>, Bsc>>());
    encodings.emplace_back(std::make_shared<Compose<StreamSplit<real>, Zstd>>());
    encodings.emplace_back(std::make_shared<Compose<StreamSplit<real>, Lz4>>());

    std::vector<std::shared_ptr<Encoding>> shortSplits;
    shortSplits.emplace_back(std::make_shared<Compose<StreamSplit<uint16_t>, Bsc>>());
    shortSplits.emplace_back(std::make_shared<Compose<StreamSplit<uint16_t>, Zstd>>());
    shortSplits.emplace_back(std::make_shared<Compose<StreamSplit<uint16_t>, Lz4>>());

    std::vector<std::shared_ptr<Method<real>>> methods;
    for (auto &e : encodings)
        methods.emplace_back(std::make_shared<Lossless<real>>(e));
    for (auto &e : encodings)
        methods.emplace_back(std::make_shared<Lfzip<real>>(e));
    for (auto &e : shortSplits)
        methods.emplace_back(std::make_shared<Lfzip<real>>(e));
    for (auto &e : encodings)
        methods.emplace_back(std::make_shared<Quantise<real>>(e));
    for (auto &e : shortSplits)
        methods.emplace_back(std::make_shared<Quantise<real>>(e));

    methods.emplace_back(std::make_shared<Machete>());
    methods.emplace_back(std::make_shared<Sz3<real>>());

    std::vector<bench_result> results;
    for (auto &m : methods)
    {
        results.emplace_back(benchmark<real>(original_buffer, *m));
    }

    Table table;
    table.add_row({"Method", "Ratio (%)", "Compression Time (ms)", "Rate (MB/s)", "Decompression Time (ms)",
                   "Rate (MB/s)", "Max Error", "MAE"});
    for (bench_result r : results)
    {
        table.add_row({r.name, string_format("%.2f", (r.compressed_size * 100.f / r.original_size)),
                       string_format("%f", r.compression_time * 1000.f), string_format("%f", r.compression_data_rate()),
                       string_format("%f", r.decompression_time * 1000.f),
                       string_format("%f", r.decompression_data_rate()), string_format("%f", r.max_error),
                       string_format("%f", r.mean_absolute_error)});
    }

    for (size_t col = 0; col < table.row(0).size(); col++)
        table[0][col].format().font_align(FontAlign::center);
    for (size_t col = 1; col < table.row(0).size(); col++)
        for (size_t row = 1; row < table.size(); row++)
            table[row][col].format().font_align(FontAlign::right);

    std::cout << table << std::endl;
    table_to_file("results.csv", table);
    return 0;
}