#pragma once

#include <memory>
#include <span>

namespace Stf {

template<typename T> struct BumpAllocator;

template<> struct BumpAllocator<std::byte> {
    using pointer = std::byte*;
    using const_pointer = const std::byte*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using value_type = std::byte;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    std::span<std::byte> m_pool;

    mutable std::mutex m_mutex {};
    size_t m_allocated = 0;
    size_t m_discarded = 0;

    template<typename T> [[nodiscard]] std::byte* allocate(size_t n) noexcept {
        const auto req_size = sizeof(T) * n;

        void* candidate_ptr = static_cast<void*>(m_pool.data());
        size_t remaining = m_pool.size();
        if (!std::align(alignof(T), req_size, candidate_ptr, remaining))
            return nullptr;
        const auto discarded = m_pool.size() - remaining;
        m_discarded += discarded;
        m_allocated += req_size;

        remaining -= req_size;

        m_pool = { m_pool.data() + discarded + req_size, remaining };

        std::byte* ret = reinterpret_cast<std::byte*>(candidate_ptr);

        return ret;
    }

    void deallocate(const std::byte*, size_t) { }
};

template<typename T> struct BumpAllocator {
    using pointer = T*;
    using const_pointer = const T*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template<typename U> struct rebind { using other = BumpAllocator<U>; };

    BumpAllocator<std::byte>& m_storage;

    [[nodiscard]] T* allocate(size_t n)
        requires(std::is_trivially_destructible_v<T>)
    {
        return reinterpret_cast<T*>(m_storage.template allocate<T>(n));
    }

    void deallocate(const T*, size_t) { }
};

// template<typename T, size_t N>
// BumpAllocator(BumpAllocatorStorage<N>& storage) -> BumpAllocator<T, N>;

}
