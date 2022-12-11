#pragma once

// macro from p0779r0
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0779r0.pdf

namespace Stf::Detail {

template<typename T> struct ExpectedReturner;

template<typename T, typename E> struct ExpectedReturner<tl::expected<T, E>> {
    constexpr static auto ret(tl::expected<T, E>&& v) { return std::move(v.value()); }
};

template<typename E> struct ExpectedReturner<tl::expected<void, E>> {
    constexpr static void ret(tl::expected<void, E>&& v) { return; }
};

}

#define TRYX(m)                                               \
    ({                                                        \
        auto res = (m);                                       \
        if (!res.has_value())                                 \
            return tl::unexpected { res.error() };            \
        Stf::Detail::ExpectedReturner<decltype(res)>::ret(std::move(res)); \
    })
