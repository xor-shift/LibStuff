#pragma once

#include <optional>
#include <utility>

#include <tl/expected.hpp>

namespace Stf::Detail {

template<typename T> struct ExpectedReturner;

template<typename T, typename E> struct ExpectedReturner<tl::expected<T, E>> {
    constexpr static T succ(tl::expected<T, E>&& v) { return std::move(v.value()); }
    constexpr static tl::unexpected<E> err(tl::expected<T, E>&& v) { return tl::unexpected { std::move(v.error()) }; }
};

template<typename E> struct ExpectedReturner<tl::expected<void, E>> {
    constexpr static void succ([[maybe_unused]] tl::expected<void, E>&&) { }
    constexpr static tl::unexpected<E> err(tl::expected<void, E>&& v) {
        return tl::unexpected { std::move(v.error()) };
    }
};

template<typename T> struct ExpectedReturner<std::optional<T>> {
    constexpr static T succ(std::optional<T>&& v) { return std::move(*v); }
    constexpr static std::nullopt_t err([[maybe_unused]] std::optional<T>&&) { return std::nullopt; }
};

}

#define TRYX(...)                                                             \
    ({                                                                        \
        auto _res = (__VA_ARGS__);                                            \
        using _returner_type = Stf::Detail::ExpectedReturner<decltype(_res)>; \
        if (!_res.has_value())                                                \
            return _returner_type::err(std::move(_res));                      \
        _returner_type::succ(std::move(_res));                                \
    })

#define TRY_OR_RET(_ret_expr, ...)                                            \
    ({                                                                        \
        auto _res = (__VA_ARGS__);                                            \
        using _returner_type = Stf::Detail::ExpectedReturner<decltype(_res)>; \
        if (!_res.has_value())                                                \
            return (_ret_expr);                                               \
        _returner_type::succ(std::move(_res));                                \
    })

#define ASSERTX(...)                                                        \
    ({                                                                      \
        auto res = (__VA_ARGS__);                                           \
        if (!res.has_value())                                               \
            std::unreachable();                                             \
        Stf::Detail::ExpectedReturner<decltype(res)>::succ(std::move(res)); \
    })
