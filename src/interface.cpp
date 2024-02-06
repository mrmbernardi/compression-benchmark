#include "benchmark.hpp"
#include "method.hpp"
#include "util.hpp"
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

template <typename F> bench_result_ex run_reconstruct(std::string method_str, void *data, int size, F error_bound)
{
    auto span = std::span<F>((F *)data, size);
    for (auto &method : get_all_methods<F>())
    {
        if (method->name() == method_str)
        {
            return benchmark<F>(span, *method, error_bound, span, true, true);
        }
    }
    throw std::runtime_error("Could not find method with name \"" + method_str + "\" for " +
                             std::to_string(sizeof(F)) + " byte float.");
}

extern "C" int reconstruct(bench_result *results, const char *method_name, char dtype, void *data, int size, double error_bound)
{
    try
    {
        std::string method_str(method_name);
        if (dtype == 'f')
        {
            *results = run_reconstruct<float>(method_str, data, size, error_bound);
        }
        else if (dtype == 'd')
        {
            *results = run_reconstruct<double>(method_str, data, size, error_bound);
        }
        else
        {
            throw std::runtime_error(std::string("Unknown data type: ") + dtype);
        }
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error." << std::endl;
    }
    return -1;
}