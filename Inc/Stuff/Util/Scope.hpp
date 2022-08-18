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
        :m_type(other.type) {
        using std::swap;
        swap(other.m_func, m_func);
    }

    ScopeGuard(GuardType type, Func&& f = {})
        : m_type(type)
        , m_func(std::forward<Func>(f)) { }

    ~ScopeGuard() noexcept(noexcept(std::invoke(std::declval<Func>()))) {
        if (!armed())
            return;

        GuardType execution_type;

        if (std::current_exception() != nullptr)
            execution_type = GuardType::ScopeFail;
        else
            execution_type = GuardType::ScopeExit;

        if (execution_type == GuardType::ScopeExit && m_type == GuardType::ScopeFail)
            return;

        std::invoke(*m_func);
    }

    constexpr void release() { m_func = std::nullopt; }
    constexpr bool armed() const { return (bool)m_func; }
    constexpr operator bool() const { return armed(); }

private:
    std::optional<Func> m_func{std::nullopt};
    GuardType m_type;
};

}
