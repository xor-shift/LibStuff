#pragma once

#include <exception>
#include <functional>
#include <optional>

namespace Stf {

enum class GuardType {
    ScopeExit,
    ScopeFail,
};

template<typename Func> struct ScopeGuard {
    ScopeGuard(ScopeGuard const&) = delete;

    ScopeGuard(ScopeGuard&& other)
        : m_type(other.type)
        , m_func(std::forward<Func>(other.m_func))
        , m_armed(other.m_armed) {
        other.release();
    }

    ScopeGuard(GuardType type, Func&& f = {})
        : m_type(type)
        , m_func(std::forward<Func>(f)) { }

    ~ScopeGuard() noexcept(noexcept(std::invoke(std::forward<Func>(m_func)))) {
        if (!armed())
            return;

        GuardType execution_type;

        if (std::current_exception() != nullptr)
            execution_type = GuardType::ScopeFail;
        else
            execution_type = GuardType::ScopeExit;

        if (execution_type == GuardType::ScopeExit && m_type == GuardType::ScopeFail)
            return;

        std::invoke(m_func);
    }

    constexpr void release() { m_armed = false; }

    constexpr bool armed() const { return m_armed; }
    constexpr operator bool() const { return armed(); }

private:
    Func&& m_func;
    GuardType m_type;
    bool m_armed = true;
};

}
