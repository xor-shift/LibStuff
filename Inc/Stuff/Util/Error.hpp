#pragma once

#include <source_location>
#include <variant>

namespace Stf {

template<typename T, typename Err> struct ErrorOr {

    constexpr ErrorOr(Err&& error)
        : m_value(std::forward<Err>(error)) {    }

    constexpr ErrorOr(Err const& error)
        : m_value(error) { }

    constexpr ErrorOr(T&& value)
        : m_value(std::forward<T>(value)) {    }

    constexpr ErrorOr(T const& value)
        : m_value(value) { }

private:
    std::variant<T, Err> m_value;
};

template<typename T> struct ErrorOr<T, void> {

    constexpr ErrorOr(T&& value)
        : m_value(std::forward<T>(value)) {    }

    constexpr ErrorOr(T const& value)
        : m_value(value) { }

private:
    std::optional<T> m_value;
};

}
