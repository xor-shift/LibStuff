#pragma once

#include <array>
#include <cstddef>
#include <utility>

#include <Stuff/Intro/HelperTypes.hpp>
#include <Stuff/Intro/Introspector.hpp>

template<typename T> struct Introspector;

#define INTRO_INDEX_FACTORY()                                                                   \
    static constexpr T& index(type& v, size_t i) { return v[i]; }                               \
    static constexpr T&& index(type&& v, size_t i) { return std::move(v[i]); }                  \
    static constexpr T const& index(type const& v, size_t i) { return v[i]; }                   \
    static constexpr T const&& index(type const&& v, size_t i) { return std::move(v[i]); }      \
    template<size_t I> static constexpr member_type& get(type& v) { return v[I]; }              \
    template<size_t I> static constexpr member_type const& get(type const& v) { return v[I]; }  \
    template<size_t I> static constexpr member_type&& get(type&& v) { return std::move(v[I]); } \
    template<size_t I> static constexpr member_type const&& get(type const&& v) { return std::move(v[I]); }

template<typename T, size_t N> struct Introspector<T[N]> {
    using type = T[N];
    using member_type = T;

    static constexpr size_t size() { return N; }
    static constexpr size_t size(type const&) { return N; }

    INTRO_INDEX_FACTORY();
};

namespace std { // NOLINT(cert-dcl58-cpp)

template<typename T> struct Introspector;

template<typename T, size_t N> struct Introspector<array<T, N>> {
    using type = array<T, N>;
    using member_type = T;

    static constexpr size_t size() { return N; }
    static constexpr size_t size(type const&) { return N; }

    INTRO_INDEX_FACTORY();
};

template<typename T, size_t N> STF_MAKE_ADL_INTROSPECTOR(array<T, N>)

}

#undef INTRO_INDEX_FACTORY
