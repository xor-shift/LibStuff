#include <benchmark/benchmark.h>

#include <fstream>
#include <random>

#include <fmt/format.h>

#include <Stuff/Graphics/Image.hpp>

#define DO_ASSERT(expr)                        \
    {                                          \
        for (bool _res = bool(expr); !_res;) { \
            std::abort();                      \
        }                                      \
    }

static void benchmark_decode_generic(benchmark::State& state, size_t width, size_t height, const char* file) {
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

static void benchmark_qoi_dice(benchmark::State& state) { benchmark_decode_generic(state, 800, 600, "Tests/Graphics/Images/dice.qoi"); }
BENCHMARK(benchmark_qoi_dice);
static void benchmark_qoi_testcard(benchmark::State& state) { benchmark_decode_generic(state, 256, 256, "Tests/Graphics/Images/testcard.qoi"); }
BENCHMARK(benchmark_qoi_testcard);

static uint8_t default_alpha_decider(uint8_t) { return 255; }

static void benchmark_encode_generic(benchmark::State& state, size_t width, size_t height, std::string_view name, auto pixel_picker) {
    std::vector<Stf::Gfx::Image<>> random_images(8, Stf::Gfx::Image(width, height));

    for (size_t i = 0; auto& image : random_images) {
        for (size_t j = 0; j < image.pixel_count(); j++) {
            auto pix = std::invoke(pixel_picker, i, j % image.dimensions()[0], j / image.dimensions()[0]);
            image.set_pixel(j, pix);
        }

        const auto filename = fmt::format("bench_{}_{}.data", name, i);
        std::ofstream ofs(filename.c_str(), std::ios::binary | std::ios::out);
        if (ofs)
            std::ranges::copy(image, std::ostreambuf_iterator<char>(ofs));

        ++i;
    }

    std::vector<uint8_t> out_res(14 + width * height * 4 + 8);

    for (auto _ : state) {
        for (auto& image : random_images) {
            auto res = Stf::Gfx::Formats::QoI::encode(out_res.begin(), image);
            DO_ASSERT(res);
        }
    }
}

static void benchmark_encode_random(benchmark::State& state) {
    std::random_device rd {};
    std::mt19937 engine {};
    std::uniform_int_distribution<uint8_t> dist {};

    auto pick_pixel = [&](size_t n, size_t x, size_t y) -> Stf::Gfx::Color { return { dist(engine), dist(engine), dist(engine), 255 }; };

    benchmark_encode_generic(state, 1024, 1024, "random", pick_pixel);
}
BENCHMARK(benchmark_encode_random);

static void benchmark_encode_patterns(benchmark::State& state) {
    auto smooth_step = [](auto x) -> decltype(x) {
        if (x >= 1)
            return 1;
        if (x <= 0)
            return 0;
        return x * x * (3 - 2 * x);
    };

    auto pick_pixel_circle = [&](double x, double y) -> Stf::Gfx::Color {
        const auto dist = std::sqrt(x * x + y * y);
        const auto ss_0 = static_cast<uint8_t>(smooth_step(dist / 128) * 255);
        const auto ss_1 = static_cast<uint8_t>(smooth_step(dist / 256) * 255);
        const auto ss_2 = static_cast<uint8_t>(smooth_step(dist / 512) * 255);

        return {ss_0, ss_1, ss_2, 255};
    };

    auto pick_pixel = [&](size_t n, size_t x, size_t y) -> Stf::Gfx::Color {
        double dx = x - 512.;
        double dy = y - 512.;

        switch (n) {
        case 0:
        default:
            return pick_pixel_circle(dx, dy);
        }
    };

    benchmark_encode_generic(state, 1024, 1024, "patterns", pick_pixel);
}
BENCHMARK(benchmark_encode_patterns);
