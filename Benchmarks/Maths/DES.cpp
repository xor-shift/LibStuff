#include <benchmark/benchmark.h>

#include <random>

#include <Stuff/Maths/Crypt/DES.hpp>

static std::random_device s_rd {};
static std::ranlux48 s_engine { s_rd() };
static std::uniform_int_distribution<uint64_t> s_gen { 0, 0xFFFF'FFFF'FFFF };

static void rng(benchmark::State& state) {
    for (auto _ : state) {
        auto v = s_gen(s_engine);
        benchmark::DoNotOptimize(v);
    }
}

BENCHMARK(rng);

static void sbox_lookup(benchmark::State& state) {
    uint64_t value = s_gen(s_engine);

    for (auto _ : state) {
        auto res = Stf::DES::Detail::sbox_transform_lookup(value++);
        benchmark::DoNotOptimize(res);
    }
}

BENCHMARK(sbox_lookup);

static void bitslice_sbox_lookup(benchmark::State& state) {
    uint64_t value = s_gen(s_engine);

    for (auto _ : state) {
        uint64_t input[48];
        uint64_t output[32] {};

        for (auto& v : input)
            v = value++;

        Stf::DES::Detail::Bitslice::X86::helper(input, output);

        for (auto& o : output)
            benchmark::DoNotOptimize(o);
    }
}

BENCHMARK(bitslice_sbox_lookup);

static void single_des(benchmark::State& state) {
    auto plaintext = s_gen(s_engine);
    auto key = s_gen(s_engine);

    for (auto _ : state) {
        auto res = Stf::DES::encrypt(plaintext++, key++);
        benchmark::DoNotOptimize(res);
    }
}

BENCHMARK(single_des);

static void crypt_3(benchmark::State& state) {
    std::string_view salt_charset =  //
        "./"                         //
        "0123456789"                 //
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ" //
        "abcdefghijklmnopqrstuvwxyz";

    auto plaintext = s_gen(s_engine);
    auto key = s_gen(s_engine);

    const char pt_begin = ' ';
    const char pt_end = '~';

    for (auto _ : state) {
        char plaintext_str[9];
        for (auto i = 0uz; i < 8uz; i++)
            plaintext_str[i] = static_cast<char>(pt_begin + (plaintext++ % (pt_end - pt_begin)));
        plaintext_str[8] = 0;

        char salt[3];
        for (auto i = 0uz; i < 2uz; i++)
            salt[i] = salt_charset[(key++) % 64];
        salt[2] = 0;

        auto res = Stf::DES::crypt(plaintext_str, salt);
        benchmark::DoNotOptimize(res);
    }
}

BENCHMARK(crypt_3);
