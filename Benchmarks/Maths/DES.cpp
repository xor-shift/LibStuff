#include <benchmark/benchmark.h>

#include <random>

#include <Stuff/Maths/Crypt/DES.hpp>

static std::random_device s_rd {};
static std::ranlux48 s_engine { s_rd() };
static std::uniform_int_distribution<uint64_t> s_gen { 0, 0xFFFF'FFFF'FFFF };

static void sbox_lookup(benchmark::State& state) {
    for (auto _ : state) {
        const auto data = s_gen(s_engine);
        auto res = Stf::Crypt::DES::Detail::sbox_transform_lookup(data);
        benchmark::DoNotOptimize(res);
    }
}

BENCHMARK(sbox_lookup);

static void single_des(benchmark::State& state) {
    const auto key = s_gen(s_engine);
    for (auto _ : state) {
        const auto plaintext = s_gen(s_engine);
        auto res = Stf::Crypt::DES::crypt(plaintext, key);
        benchmark::DoNotOptimize(res);
    }
}

BENCHMARK(single_des);
