#pragma once

#include <functional>
#include <tuple>
#include <utility>

namespace Stf {

namespace Detail {

template<typename T, typename U> struct TupleCombiner;

template<typename... Ts, typename... Us> struct TupleCombiner<std::tuple<Ts...>, std::tuple<Us...>> {
    using type = std::tuple<Ts..., Us...>;
};

template<typename T, size_t N> struct TupleGenerator;

template<typename T> struct TupleGenerator<T, 0> {
    using type = std::tuple<>;
};

template<typename T> struct TupleGenerator<T, 1> {
    using type = std::tuple<T>;
};

template<typename T, size_t N> struct TupleGenerator {
    using lhs_type = typename TupleGenerator<T, N / 2>::type;
    using rhs_type = typename TupleGenerator<T, N / 2 + N % 2>::type;

    using type = typename TupleCombiner<lhs_type, rhs_type>::type;
};

}

template<typename T, typename U> using tuple_combine_t = typename Detail::TupleCombiner<T, U>::type;

template<typename T, size_t N> using n_tuple_t = typename Detail::TupleGenerator<T, N>::type;

template<typename T, T N, T... Ns>
constexpr std::integer_sequence<T, Ns...> integer_sequence_cdr(std::integer_sequence<T, N, Ns...>) {
    return {};
}

template<size_t... Ns, typename... Ts>
constexpr auto tuple_extract(std::tuple<Ts...>&& tuple, std::integer_sequence<size_t, Ns...>) {
    return std::tuple(std::move(std::get<Ns>(tuple))...);
}

template<typename... Ts> constexpr auto tuple_cdr(std::tuple<Ts...>&& tuple) {
    auto seq_base = std::make_index_sequence<sizeof...(Ts)> {};
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
        auto bound_fn = [&fn, car](Args&&... args) mutable { return fn(std::move(car), std::forward<Args>(args)...); };
        return tuple_call(std::move(bound_fn), std::move(rest));
    }
}

}
