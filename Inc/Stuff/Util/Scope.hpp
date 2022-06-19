#pragma once

#include <exception>
#include <functional>

namespace Stf {

enum class GuardType {
    ScopeExit,
    ScopeFail,
};

template<typename Func> struct ScopeGuard {
    ScopeGuard(GuardType type, Func&& f)
        : m_type(type)
        , m_func(std::forward<Func>(f)) { }

    ~ScopeGuard() noexcept(noexcept(std::invoke(m_func))) {
        if (!m_armed)
            return;

        GuardType execution_type;

        if (std::current_exception() != nullptr)
            execution_type = GuardType::ScopeFail;
        else
            execution_type = GuardType::ScopeExit;

        if (m_type != execution_type)
            return;

        std::invoke(m_func);
    }

    constexpr void disarm() { m_armed = false; }
    constexpr void arm() { m_armed = true; }
    constexpr bool armed() const { return m_armed; }
    constexpr operator bool() const { return armed(); }

private:
    bool m_armed = true;
    GuardType m_type;
    Func m_func;
};

}
