#include "gtest/gtest.h"

#include <Stuff/Util/Alloc.hpp>

#include <vector>

TEST(Alloc, Alloc) {
    std::array<std::byte, 10240> container {};
    Stf::BumpAllocatorStorage<decltype(container)> storage { container };
    Stf::BumpAllocator<size_t, decltype(container)> alloc { storage };

    auto* a = alloc.allocate(1);
    auto* b = alloc.allocate(1);
    auto* c = alloc.allocate(1);
    auto* d = alloc.allocate(1);
    auto* e = alloc.allocate(4);
    auto* f = alloc.allocate(4);

    std::vector<size_t, decltype(alloc)> vec(alloc);

    for (size_t i = 0; i < 64; i++) {
        vec.emplace_back(i);
    }

    std::ignore = vec;
}