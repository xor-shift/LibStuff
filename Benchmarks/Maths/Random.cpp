#include <benchmark/benchmark.h>

#include <Stuff/Maths/Random.hpp>

static void normal_bm(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::norm_impl_bm<float>();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(normal_bm);

static void normal_mp(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::norm_impl_mp<float>();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(normal_mp);

static void normal_std(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::norm_impl_std<float>();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(normal_std);

static void sphere_0_snorm(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::SphereSamplers<float, 0>::snorm_range();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(sphere_0_snorm);

static void sphere_1_polar(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::SphereSamplers<float, 1>::polar();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(sphere_1_polar);

static void sphere_1_rejection(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::SphereSamplers<float, 1>::rejection();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(sphere_1_rejection);

static void sphere_1_gaussian(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::SphereSamplers<float, 1>::gaussian();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(sphere_1_gaussian);

static void ball_1_snorm(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::BallSamplers<float, 1>::snorm();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(ball_1_snorm);

static void ball_2_rejection(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::BallSamplers<float, 2>::rejection();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(ball_2_rejection);

static void ball_2_polar_radial(benchmark::State& state) {
    for (auto _ : state) {
        const auto sample = Stf::RNG::Detail::BallSamplers<float, 2>::polar_radial();
        benchmark::DoNotOptimize(sample);
    }
}
BENCHMARK(ball_2_polar_radial);