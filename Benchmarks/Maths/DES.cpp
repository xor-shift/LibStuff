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
    const auto plaintext = s_gen(s_engine);
    for (auto _ : state) {
        const auto key = s_gen(s_engine);
        auto res = Stf::Crypt::DES::encrypt(plaintext, key);
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

    std::uniform_int_distribution<size_t> salt_gen { 0, 64 };
    std::uniform_int_distribution<char> plaintext_gen { ' ', '~' };

    for (auto _ : state) {
        char plaintext_str[9];
        for (auto i = 0uz; i < 8uz; i++)
            plaintext_str[i] = plaintext_gen(s_engine);
        plaintext_str[8] = 0;

        char salt[3];
        for (auto i = 0uz; i < 2uz; i++)
            salt[i] = salt_charset[salt_gen(s_engine)];
        salt[2] = 0;

        const auto key = s_gen(s_engine);
        auto res = Stf::Crypt::DES::crypt(plaintext_str, salt);
        benchmark::DoNotOptimize(res);
    }
}

BENCHMARK(crypt_3);
