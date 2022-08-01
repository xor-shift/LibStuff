#pragma once

#include <cmath>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace Stf {

namespace Detail {

template<typename Func, typename Clock> struct TimerBase {
    TimerBase(std::chrono::milliseconds every, Func&& f = {})
        : m_func(std::forward<Func>(f)) {
        start(every);
    }

    virtual ~TimerBase() { cancel(); }

    void start(std::chrono::milliseconds every) {
        if (m_armed)
            return;

        m_started_at = Clock::now();
        m_every = every;
        m_armed = true;
    }

    void cancel() {
        if (!m_armed)
            return;

        m_armed.store(false);

        std::unique_lock lock(m_cv_mutex);
        m_cv.notify_all();
    }

    void run() {
        while (m_armed) {
            std::unique_lock lock(m_cv_mutex);
            m_last_wait_start = std::chrono::system_clock::now();

            m_cv.wait_until(lock, (m_next_tick_at = next_tick_at()), [this] { return !m_armed.load() || duration_is_sufficient(); });

            if (!m_armed)
                break;

            std::invoke(m_func);
        }
    }

protected:
    virtual bool duration_is_sufficient() { return m_next_tick_at <= Clock::now(); }

    virtual std::chrono::time_point<Clock> next_tick_at() = 0;

    std::chrono::time_point<Clock> m_last_wait_start {};
    std::chrono::time_point<Clock> m_next_tick_at {};
    std::chrono::time_point<Clock> m_started_at {};
    std::chrono::milliseconds m_every {};

private:
    Func m_func;

    std::atomic_bool m_armed { false };
    std::mutex m_cv_mutex {};
    std::condition_variable m_cv {};
};

}

template<typename Func, typename Clock = std::chrono::system_clock> struct SleepTimer final : public Detail::TimerBase<Func, Clock> {
    using base_type = Detail::TimerBase<Func, Clock>;

    virtual ~SleepTimer() = default;

    SleepTimer() = default;

    SleepTimer(std::chrono::milliseconds every, Func&& f = {})
        : base_type(every, std::forward<Func>(f)) { }

protected:
    std::chrono::time_point<std::chrono::system_clock> next_tick_at() final override {
        const auto now = std::chrono::system_clock::now();
        return now + base_type::m_every;
    }
};

template<typename Func, typename Clock = std::chrono::system_clock> struct AccurateTimer final : public Detail::TimerBase<Func, Clock> {
    using base_type = Detail::TimerBase<Func, Clock>;

    virtual ~AccurateTimer() = default;

    AccurateTimer() = default;

    AccurateTimer(std::chrono::milliseconds every, Func&& f = {})
        : base_type(every, std::forward<Func>(f)) { }

protected:
    std::chrono::time_point<std::chrono::system_clock> next_tick_at() final override {
        const auto from = std::chrono::duration_cast<std::chrono::milliseconds>(base_type::m_started_at.time_since_epoch()).count();
        const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(base_type::m_last_wait_start.time_since_epoch()).count();

        const auto passed_cnt = std::floor(static_cast<long double>(now - from) / static_cast<long double>(base_type::m_every.count()));
        const auto wanted_cnt = static_cast<decltype(from)>(passed_cnt) + 1;
        const auto wanted_dur = base_type::m_every * wanted_cnt;
        const auto tp = base_type::m_started_at + wanted_dur;

        return tp;
    }
};

}
