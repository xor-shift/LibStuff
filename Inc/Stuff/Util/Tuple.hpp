#pragma once

#include <tuple>
#include <utility>

namespace Stf {

template<typename T, T N, T... Ns>
constexpr std::integer_sequence<T, Ns...> integer_sequence_cdr(std::integer_sequence<T, N, Ns...>) {
    return {};
}

template<size_t... Ns, typename... Ts>
constexpr auto tuple_extract(std::tuple<Ts...>&& tuple, std::integer_sequence<size_t, Ns...>) {
    return std::tuple(std::move(std::get<Ns>(tuple))...);
}

template<typename... Ts>
constexpr auto tuple_cdr(std::tuple<Ts...>&& tuple) {
    auto seq_base = std::make_index_sequence<sizeof...(Ts)>{};
    auto seq_cdr = integer_sequence_cdr(seq_base);
    return tuple_extract(std::move(tuple), seq_cdr);
}

template<typename Fn, typename Arg, typename... Args>
constexpr auto tuple_call(Fn&& fn, std::tuple<Arg, Args...> args) {
    auto car = std::move(std::get<0>(args));

    if constexpr (sizeof...(Args) == 0) {
        return std::invoke(fn, std::move(car));
    } else {
        auto rest = tuple_cdr(std::move(args));
        auto bound_fn = [&fn, car](Args&&... args) { return fn(std::move(car), std::forward<Args...>(args)...); };
        return tuple_call(std::move(bound_fn), std::move(rest));
    }
}

}
