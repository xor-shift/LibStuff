#pragma once

#include <tuple>
#include <utility>

#include <Stuff/Intro/Introspector.hpp>
#include <Stuff/Intro/HelperTypes.hpp>

#define INTRO_GET_FACTORY()                                                                                  \
    template<size_t I> static constexpr nth_type<I>& get(type& v) { return std::get<I>(v); }                                  \
    template<size_t I> static constexpr nth_type<I> const& get(type const& v) { return std::get<I>(v); }                      \
    template<size_t I> static constexpr nth_type<I>&& get(type&& v) { return std::move(std::get<I>(std::forward<type>(v))); } \
    template<size_t I> static constexpr nth_type<I> const&& get(type const&& v) { return std::move(std::get<I>(std::forward<type>(v))); }

namespace std { // NOLINT(cert-dcl58-cpp)

template<typename T> struct Introspector;

template<typename T, typename U> struct Introspector<pair<T, U>> {
    using type = pair<T, U>;
    using types = ::Stf::ABunchOfTypes<T, U>;
    template<size_t I> using nth_type = tuple_element_t<I, types>;

    static constexpr size_t size() { return 2; };
    static constexpr size_t size(pair<T, U> const&) { return size(); };

    INTRO_GET_FACTORY();
};

template<typename T, typename U> STF_MAKE_ADL_INTROSPECTOR(pair<T, U>)

template<typename... Ts> struct Introspector<tuple<Ts...>> {
    using type = tuple<Ts...>;
    using types = ::Stf::ABunchOfTypes<Ts...>;
    template<size_t I> using nth_type = tuple_element_t<I, types>;

    static constexpr size_t size() { return sizeof...(Ts); }
    static constexpr size_t size(tuple<Ts...> const&) { return size(); };

    INTRO_GET_FACTORY();
};

template<typename... Ts> STF_MAKE_ADL_INTROSPECTOR(tuple<Ts...>)

}

#undef INTRO_GET_FACTORY
