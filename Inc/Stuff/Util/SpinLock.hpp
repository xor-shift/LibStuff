#pragma once

#include <atomic>

#if !defined(STF_SPINLOCK_USE_STD_YIELD) && !defined(STF_SPINLOCK_USE_SSE_YIELD)
#    define STF_SPINLOCK_USE_STD_YIELD
#endif

#ifdef STF_SPINLOCK_USE_STD_YIELD
#    include <thread>
#    define STF_YIELD std::this_thread::yield
#else
#    include <immintrin.h>
#    define STF_YIELD _mm_pause
#endif

namespace Stf {

struct SpinMutex {
    void lock() {
        for (;;) {
            // if the former value is false, we have locked successfully
            if (!m_impl.exchange(true, std::memory_order_acquire))
                return;

            // while the mutex is locked, spin
            while (m_impl.load(std::memory_order_relaxed))
                STF_YIELD();
        }
    }

    bool try_lock() { return !m_impl.load(std::memory_order_relaxed) && !m_impl.exchange(true, std::memory_order_acquire); }

    void unlock() { m_impl.store(false, std::memory_order_release); }

private:
    std::atomic_bool m_impl { false };
};

}

#undef STF_SPINLOCK_USE_STD_YIELD
#undef STF_SPINLOCK_USE_SSE_YIELD
#undef STF_YIELD
