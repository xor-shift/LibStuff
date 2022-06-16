#include <benchmark/benchmark.h>

#include <fstream>

#include <Stuff/Graphics/Image.hpp>

#define DO_ASSERT(expr)                        \
    {                                          \
        for (bool _res = bool(expr); !_res;) { \
            std::abort();                      \
        }                                      \
    }

static void benchmark_qoi_generic(benchmark::State& state, size_t width, size_t height, const char* file) {
    std::ifstream image_ifs(file, std::ios::binary | std::ios::in);
    DO_ASSERT(image_ifs);

    std::vector<uint8_t> qoi_data(std::istreambuf_iterator<char> { image_ifs }, std::istreambuf_iterator<char>());
    Stf::Gfx::Image image(width, height);

    for (auto _ : state) {
        const auto res = Stf::Gfx::Formats::QoI::decode(qoi_data.begin(), qoi_data.end(), image);
#ifndef NDEBUG
        DO_ASSERT(res)
#endif
    }
}

static void benchmark_qoi_dice(benchmark::State& state) { benchmark_qoi_generic(state, 800, 600, "Tests/Graphics/Images/dice.qoi"); }
BENCHMARK(benchmark_qoi_dice);
static void benchmark_qoi_testcard(benchmark::State& state) { benchmark_qoi_generic(state, 256, 256, "Tests/Graphics/Images/testcard.qoi"); }
BENCHMARK(benchmark_qoi_testcard);

BENCHMARK_MAIN();
