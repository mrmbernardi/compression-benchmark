#include "benchmark.hpp"
#include "tabulate/font_align.hpp"
#include "tabulate/table.hpp"
#include "util.hpp"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

using namespace tabulate;

typedef float real;
// typedef double real;

template <typename F> class Method;

extern "C" int reconstruct(bench_result *results, const char *method_name, char dtype, void *data, int size, double error_bound);

int main(int argc, char **argv)
{
    // bench_result r;
    // std::array<double, 1> x = {-364.52299570321952};
    // return reconstruct(&r, "Sz3", 'd', x.data(), x.size(), 1.0);

    for (int i = 0; i < argc; i++)
    {
        if (std::string(argv[i]) == "--names")
        {
            for (auto &s : get_all_names())
                std::cout << s << std::endl;
            return 0;
        }
    }

    std::vector<real> original_buffer = generate_random_data<real>(65536, -500, 500); // 20000000);
    //  vec_to_file("data.vec", original_buffer);
    // std::vector<real> original_buffer = vec_from_file<double>("/home/bem@PADNT/fpc/bin/obs_temp.trace.fpc.bin");
    // bench_result res;
    // reconstruct(&res, "LfZip with Stream Split (V) with Lz4", 'd', void *data, original_buffer.size(), 1e-6);
    // return 0;

    std::vector<bench_result_ex> results;
    auto methods = get_all_methods<real>();
    // reversing and popping back ensures the buffers in the encoders are freed as it goes which seems to give better
    // performance.
    std::reverse(methods.begin(), methods.end());
    while(!methods.empty())
    {
        results.emplace_back(benchmark<real>(original_buffer, *methods.back(), 1.0));
        methods.pop_back();
    }

    Table table;
    table.add_row({"Method", "Ratio (%)", "Compression Time (ms)", "Rate (MB/s)", "Decompression Time (ms)",
                   "Rate (MB/s)", "Max Error", "MAE"});
    for (bench_result_ex r : results)
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