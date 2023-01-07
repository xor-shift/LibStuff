#pragma once

#include <cstddef>

#include <Stuff/Intro/AggregateExplorers.hpp>
#include <Stuff/Intro/HelperTypes.hpp>
#include <Stuff/Util/Tuple.hpp>

namespace Stf {

/// A simple aggregate; an aggregate without any reference types nor n of T types
template<typename T>
concept SAgg = std::is_aggregate_v<T> && (!std::is_array_v<T>) && std::is_default_constructible_v<T>;

/// A simple aggregate that can be initialized with rvalue references to Args...
/// @related
/// Concept SAgg\<T\>
template<typename T, typename... Args>
concept SAggOf = SAgg<T> && requires(Args&&... args) { T { std::forward<Args>(args)... }; };

/// Initializes a simple aggregate with a tuple of rvalue references to Args...
/// @related
/// Concept SAggOf\<T, Args...\>
template<SAgg T, typename... Args>
    requires(SAggOf<T, Args...>)
constexpr T sagg_init(std::tuple<Args&&...> args) {
    auto lambda = [](Args&&... args) -> T { return T { args... }; };

    return tuple_call(lambda, std::move(args));
}

/// Initializes a simple aggregate with a tuple of rvalue references to Args...
/// @related
/// Concept SAggOf\<T, Args...\>
template<SAgg T, typename... Args>
    requires(SAggOf<T, Args...> && std::is_move_assignable_v<T>)
constexpr void sagg_init(T& dst, std::tuple<Args&&...> args) {
    auto lambda = [&dst](Args&&... args) -> T {
        T src { args... };
        dst = std::move(src);
    };

    return tuple_call(lambda, std::move(args));
}

/// simple aggregate that can be initialised with N or less arguments
template<typename T, size_t N>
concept SAggOfN = SAgg<T> && requires { sagg_init<T>(std::declval<n_tuple_t<ConvertibleToAnything&&, N>>()); };

namespace Detail {

/// An aggregate's arity is less than this value
template<typename T, size_t LowerBound = 0> static constexpr size_t sagg_arity_upper_bound() {
    constexpr size_t next_index = LowerBound == 0 ? 1 : LowerBound * 2;
    if constexpr (!SAggOfN<T, next_index>) {
        return next_index;
    } else {
        return sagg_arity_upper_bound<T, next_index>();
    }
}

template<typename T, size_t low, size_t high> static constexpr size_t sagg_arity_log() {
    if constexpr (high - low == 1)
        return low;

    constexpr size_t mid = (high + low) / 2;
    constexpr bool can = SAggOfN<T, mid>;

    if constexpr (can)
        return sagg_arity_log<T, mid, high>();
    else
        return sagg_arity_log<T, low, mid>();
}

template<typename T, size_t N> static constexpr size_t sagg_arity_linear() {
    constexpr bool can = SAggOfN<T, N>;

    if constexpr (can)
        return sagg_arity_linear<T, N + 1>();
    else
        return N - 1;
}

template<typename T> static constexpr size_t sagg_arity() {
    constexpr bool default_initialisable = SAggOfN<T, 0>;

    if constexpr (default_initialisable) {
        // because sagg_arity_log returns the low-point
        constexpr bool empty = !SAggOfN<T, 1>;
        if constexpr (empty)
            return 0;

        return sagg_arity_log<T, 1, sagg_arity_upper_bound<T, 2>()>();
    } else
        return sagg_arity_linear<T, 2>();
}

}

template<SAgg T> inline constexpr size_t sagg_arity = Detail::sagg_arity<T>();
template<SAgg T, size_t N = sagg_arity<T>>
using sagg_types = decltype(Stf::Detail::aggregate_member_helper<T>(std::integral_constant<size_t, N> {}));

template<SAgg T> struct SAggIntrospector {
    using type = std::remove_cvref_t<T>;
    using types = sagg_types<T>;
    template<size_t I> using nth_type = std::tuple_element_t<I, types>;

    static constexpr size_t size() { return sagg_arity<T>; }
    static constexpr size_t size(T const&) { return size(); }

    template<size_t I, typename U>
        requires(std::is_same_v<type, std::remove_cvref_t<U>>)
    static constexpr decltype(auto) get(U&& v) {
        return Detail::explore_aggregate<size()>(v, [](auto f) -> decltype(auto) { return std::get<I>(f); });
    }
};

}
