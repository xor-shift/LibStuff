#include <benchmark/benchmark.h>

#include <fstream>

#include <Stuff/Graphics/Image.hpp>

#define STF_ASSERT(expr)                     \
    {                                        \
        for (bool res = bool(expr); !res;) { \
            std::abort();                    \
        }                                    \
    }

static void benchmark_qoi_decode_span(benchmark::State& state) {
    std::ifstream image_ifs("Tests/Graphics/Images/testcard.qoi", std::ios::binary | std::ios::in);
    STF_ASSERT(image_ifs);

    std::vector<uint8_t> qoi_data(std::istreambuf_iterator<char>{image_ifs}, std::istreambuf_iterator<char>());
    Stf::Gfx::Image image(256, 256);
    for (auto _ : state) {
        Stf::Gfx::Formats::QoI::decode_into(std::span(qoi_data), image);
    }
}

BENCHMARK(benchmark_qoi_decode_span);

static void benchmark_qoi_decode_inputiter(benchmark::State& state) {
    std::ifstream image_ifs("Tests/Graphics/Images/testcard.qoi", std::ios::binary | std::ios::in);
    STF_ASSERT(image_ifs);

    std::vector<uint8_t> qoi_data(std::istreambuf_iterator<char>{image_ifs}, std::istreambuf_iterator<char>());
    Stf::Gfx::Image image(256, 256);
    for (auto _ : state) {
        Stf::Gfx::Formats::QoI::decode_into(qoi_data.begin(), qoi_data.end(), image);
    }
}

BENCHMARK(benchmark_qoi_decode_inputiter);


BENCHMARK_MAIN();
