#pragma once

#include <memory>

namespace Stf {

template<typename Container> struct BumpAllocatorStorage {
    BumpAllocatorStorage(Container& container)
        :m_container(container) { reset(true); }

    template<typename T> [[nodiscard]] std::byte* allocate(size_t n) noexcept {
        const auto req_size = sizeof(T) * n;

        void* candidate_ptr = static_cast<void*>(m_p);
        if (!std::align(alignof(T), req_size, candidate_ptr, m_remaining))
            return nullptr;

        std::byte* ret = reinterpret_cast<std::byte*>(candidate_ptr);
        m_remaining -= req_size;

        m_allocated += req_size;
        m_discarded += std::distance(m_p, ret);

        m_p = ret + req_size;

        return ret;
    }

    void reset(bool reset_stats = false) {
        m_p = m_container.data();
        m_remaining = m_container.size();

        if (reset_stats) {
            m_allocated = 0;
            m_discarded = 0;
        }
    }

private:
    Container& m_container;

    std::byte* m_p = nullptr;
    size_t m_remaining = 0;

    // statistics
    size_t m_allocated = 0;
    size_t m_discarded = 0;
};

template<typename T, typename Container> struct BumpAllocator {
    using pointer = T*;
    using const_pointer = const T*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template<typename U> struct rebind { using other = BumpAllocator<U, Container>; };

    BumpAllocator(BumpAllocatorStorage<Container>& storage) noexcept
        : m_storage(storage) { }

    [[nodiscard]] T* allocate(size_t n)
        requires(std::is_trivially_destructible_v<T>)
    {
        return reinterpret_cast<T*>(m_storage.template allocate<T>(n));
    }

    void deallocate(const T*, size_t) { }

private:
    BumpAllocatorStorage<Container>& m_storage;
};

// template<typename T, size_t N>
// BumpAllocator(BumpAllocatorStorage<N>& storage) -> BumpAllocator<T, N>;

}
