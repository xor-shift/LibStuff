#include <benchmark/benchmark.h>

#include <Stuff/Maths/Hash/Sha2.hpp>

#include <random>

template<typename Props> static void sha2_generic(benchmark::State& state) {
    std::random_device rd {};
    std::mt19937 rng(rd());

    std::array<uint32_t, 4096> data;
    std::generate(begin(data), end(data), rng);

    Stf::Hash::SHA2::SHA2State<Props> hasher {};

    for (auto _ : state) {
        hasher.update(std::span<uint32_t>(data));
        auto digest = hasher.finish();

        benchmark::DoNotOptimize(digest);
    }
}

static void sha2_224(benchmark::State& state) { return sha2_generic<Stf::Hash::SHA2::SHA224Properties>(state); }
BENCHMARK(sha2_224);
static void sha2_256(benchmark::State& state) { return sha2_generic<Stf::Hash::SHA2::SHA256Properties>(state); }
BENCHMARK(sha2_256);
static void sha2_384(benchmark::State& state) { return sha2_generic<Stf::Hash::SHA2::SHA384Properties>(state); }
BENCHMARK(sha2_384);
static void sha2_512(benchmark::State& state) { return sha2_generic<Stf::Hash::SHA2::SHA512Properties>(state); }
BENCHMARK(sha2_512);
