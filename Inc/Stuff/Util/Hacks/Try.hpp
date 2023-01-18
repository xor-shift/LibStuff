#pragma once

#include <utility>

#include <tl/expected.hpp>

namespace Stf::Detail {

template<typename T> struct ExpectedReturner;

template<typename T, typename E> struct ExpectedReturner<tl::expected<T, E>> {
    constexpr static auto ret(tl::expected<T, E>&& v) { return std::move(v.value()); }
};

template<typename E> struct ExpectedReturner<tl::expected<void, E>> {
    constexpr static void ret([[maybe_unused]] tl::expected<void, E>&&) { }
};

}

#define TRYX(...)                                                            \
    ({                                                                       \
        auto _res = (__VA_ARGS__);                                           \
        if (!_res.has_value())                                               \
            return tl::unexpected { _res.error() };                          \
        Stf::Detail::ExpectedReturner<decltype(_res)>::ret(std::move(_res)); \
    })

#define ASSERTX(...)                                                       \
    ({                                                                     \
        auto res = (__VA_ARGS__);                                          \
        if (!res.has_value())                                              \
            std::unreachable();                                            \
        Stf::Detail::ExpectedReturner<decltype(res)>::ret(std::move(res)); \
    })
