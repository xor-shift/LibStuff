#pragma once

#include <source_location>
#include <variant>

namespace Stf {

template<typename T, typename Err> struct Error : public std::variant<T, Err> {
    template<typename... Us>
    constexpr Error(Us&&... v)
        : std::variant<T, Err>(std::forward<Us>(v)...) { }

    constexpr bool is_error() const { return std::holds_alternative<Err>(*this); }
    constexpr bool has_value() const { return std::holds_alternative<T>(*this); }
    constexpr operator bool() const { return has_value(); }

    constexpr T const& value() const& { return std::get<T>(*this); }
    constexpr T& value() & { return std::get<T>(*this); }
    constexpr T const&& value() && { return std::move(std::get<T>(*this)); }
    constexpr T&& value() const&& { return std::move(std::get<T>(*this)); }

    constexpr Err const& error() const& { return std::get<Err>(*this); }
    constexpr Err& error() & { return std::get<Err>(*this); }
    constexpr Err const&& error() && { return std::move(std::get<Err>(*this)); }
    constexpr Err&& error() const&& { return std::move(std::get<Err>(*this)); }

    constexpr const T* operator->() const noexcept { return std::addressof(value()); }
    constexpr T* operator->() noexcept { return std::addressof(value()); }
    constexpr const T& operator*() const& noexcept { return value(); }
    constexpr T& operator*() & noexcept { return value(); }
    constexpr const T&& operator*() const&& noexcept { return std::move(value()); }
    constexpr T&& operator*() && noexcept { return std::move(value()); }

    // template<typename U> constexpr T value_or(U&& v) const& { return has_value() ? }
};

#define unwrap(to, src_expr)      \
    if (auto res = src_expr; res) \
        to = *res;                \
    else                          \
        for (;;)                  \
            std::abort();

#define unwrap_or_return(to, src_expr, ret_expr) \
    if (auto res = src_expr; res)                \
        to = *res;                               \
    else                                         \
        return ret_expr
}

#ifdef GOOGLETEST_INCLUDE_GTEST_GTEST_H_

#    define ASSERT_NO_ERROR(expr)                                                                                \
        if (expr.is_error()) {                                                                                   \
            std::cerr << "ASSERT_NO_ERROR for " << #expr << " failed with error: " << expr.error() << std::endl; \
            ASSERT_TRUE(expr);                                                                                   \
        }

#endif
