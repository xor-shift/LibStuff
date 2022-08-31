#pragma once

#include "./Concepts.hpp"

namespace Stf::Detail {

template<typename T0, typename T1, typename... Ts> constexpr bool generic_compat_checker(const auto checker = {}) {
    constexpr bool cur_compat = checker.template operator()<T0, T1>();

    if constexpr (sizeof...(Ts) == 0)
        return cur_compat;
    else
        return cur_compat && generic_compat_checker<T1, Ts...>(checker);
}

template<typename Oper, typename Indexer, typename Tuple, size_t N = 0,
    typename T = typename std::tuple_element_t<0, Tuple>::value_type>
constexpr auto fold(size_t i, Tuple const& tuple, Oper const& op = {}, Indexer&& indexer = {}, T prev = {}) {
    constexpr size_t max_n = std::tuple_size_v<Tuple>;
    const auto cur = static_cast<T>(std::invoke(indexer, std::get<N>(tuple), i));

    if constexpr (max_n == 0) {
        return cur;
    }

    if constexpr (N == 0) {
        return fold<Oper, Indexer, Tuple, N + 1>(i, tuple, op, std::forward<Indexer>(indexer), cur);
    } else {
        const auto res = static_cast<T>(std::invoke(op, prev, cur));
        if constexpr (N + 1 == max_n)
            return res;
        else
            return fold<Oper, Indexer, Tuple, N + 1>(i, tuple, op, std::forward<Indexer>(indexer), cur);
    }
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression... Es> struct VariadicVectorHelper {
    using value_type = typename E0::value_type;
    static constexpr size_t vector_size = E0::vector_size;

    static constexpr bool is_compatible = generic_compat_checker<E0, Es...>(
        []<Concepts::VectorExpression U, Concepts::VectorExpression V>() { return U::vector_size == V::vector_size; });
};

template<Concepts::VectorExpression... Es>
constexpr bool vectors_are_compatible() { return VariadicVectorHelper<Es...>::is_compatible; }

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression... Es> struct VariadicMatrixHelper {
    using value_type = typename E0::value_type;
    static constexpr size_t rows = E0::rows;
    static constexpr size_t cols = E0::cols;

    static constexpr bool is_compatible
        = generic_compat_checker<E0, Es...>([]<Concepts::MatrixExpression U, Concepts::MatrixExpression V>() {
              return U::rows == V::rows && U::cols == V::cols;
          });
};

}
