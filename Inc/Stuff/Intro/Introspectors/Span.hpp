#pragma once

#include <span>

#include <Stuff/Intro/Introspector.hpp>

#define INTRO_INDEX_FACTORY()                                                                   \
    static constexpr T& index(type& v, size_t i) { return v[i]; }                               \
    static constexpr T&& index(type&& v, size_t i) { return std::move(v[i]); }                  \
    static constexpr T const& index(type const& v, size_t i) { return v[i]; }                   \
    static constexpr T const&& index(type const&& v, size_t i) { return std::move(v[i]); }

namespace std { // NOLINT(cert-dcl58-cpp)

template<typename T> struct Introspector;

template<typename T> struct Introspector<span<T>> {
    using type = span<T>;
    using member_type = T;

    static constexpr size_t size(type const& v) { return v.size(); }

    INTRO_INDEX_FACTORY();
};

template<typename T> STF_MAKE_ADL_INTROSPECTOR(span<T>)

}

#undef INTRO_INDEX_FACTORY