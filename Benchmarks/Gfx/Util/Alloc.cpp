#include <benchmark/benchmark.h>

#include <array>
#include <random>
#include <unordered_map>

#include <Stuff/Util/Alloc.hpp>

static std::array<std::byte, 1024 * 1024 * 512> s_bump_allocator_container;

static std::random_device s_random_device {};
static std::mt19937_64 s_random_generator { s_random_device() };

template<typename Allocator> static void allocate_single_size(Allocator const& alloc_ref = {}) {
    Allocator alloc { alloc_ref };

    using value_type = typename Allocator::value_type;
    using pointer_type = value_type*;

    static const size_t block_count = 1024;
    static const size_t block_size = s_bump_allocator_container.size() / (block_count * sizeof(value_type));

    std::array<pointer_type, block_count> pointers;

    for (size_t i = 0; i < block_count; i++)
        pointers[i] = alloc.allocate(1);

    benchmark::DoNotOptimize(pointers);

    for (auto* p : pointers)
        alloc.deallocate(p, 1);
}

static void benchmark_bump_single_size(benchmark::State& state) {
    Stf::BumpAllocator<std::byte> storage {};
    Stf::BumpAllocator<int> alloc { storage };
    for (auto _ : state) {
        storage.m_pool = s_bump_allocator_container;
        allocate_single_size(alloc);
    }
}
BENCHMARK(benchmark_bump_single_size);

static void benchmark_std_single_size(benchmark::State& state) {
    std::allocator<int> alloc {};

    for (auto _ : state) {
        allocate_single_size(alloc);
    }
}
BENCHMARK(benchmark_std_single_size);

template<typename Allocator> static void allocate_mixed_sizes(Allocator const& alloc_ref = {}) {
    Allocator alloc { alloc_ref };

    using value_type = typename Allocator::value_type;
    using pointer_type = value_type*;

    const auto max_allocations = std::min<size_t>(1024 * 4, s_bump_allocator_container.size() / sizeof(value_type));
    size_t left_to_consume = max_allocations;

    std::array<std::pair<pointer_type, size_t>, max_allocations> pointers;
    std::uniform_int_distribution<size_t> dist(32, 256);

    for (size_t i = 0; left_to_consume > 0; i++) {
        const auto sz = std::min(dist(s_random_generator), left_to_consume);
        left_to_consume -= sz;

        pointers[i] = { alloc.allocate(sz), sz };
    }

    benchmark::DoNotOptimize(pointers);

    for (auto const& [p, i] : pointers)
        alloc.deallocate(p, i);
}

static void benchmark_bump_mixed_size(benchmark::State& state) {
    Stf::BumpAllocator<std::byte> storage {};
    Stf::BumpAllocator<int> alloc { storage };
    for (auto _ : state) {
        storage.m_pool = s_bump_allocator_container;
        allocate_mixed_sizes(alloc);
    }
}
BENCHMARK(benchmark_bump_mixed_size);

static void benchmark_std_mixed_size(benchmark::State& state) {
    std::allocator<int> alloc {};

    for (auto _ : state) {
        allocate_mixed_sizes(alloc);
    }
}
BENCHMARK(benchmark_std_mixed_size);