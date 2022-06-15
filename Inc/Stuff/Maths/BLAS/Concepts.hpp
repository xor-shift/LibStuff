#pragma once

#include <concepts>
#include <cstddef>
#include <functional>

namespace Stf::Concepts {

template<typename T>
concept VectorExpression = requires(T const& cv, size_t i) {
                               { cv[i] } -> std::convertible_to<typename T::value_type>;
                               { T::vector_size } -> std::convertible_to<size_t>;
                           };

template<typename T>
concept MatrixExpression = requires(T const& self, size_t i, size_t j) {
                               // typename T::value_type;
                               { self.at(i, j) } -> std::convertible_to<typename T::value_type>;
                               { T::cols } -> std::convertible_to<size_t>;
                               { T::rows } -> std::convertible_to<size_t>;
                           };

}

namespace Stf::Detail {

template<typename T, typename E> struct VectorExpression {
    using value_type = T;
    constexpr static size_t vector_size = E::vector_size;

    constexpr value_type operator[](size_t i) const { return static_cast<E const*>(this)->operator[](i); }
};

template<typename T, typename E> struct MatrixRowExpression;

template<typename T, typename E> struct MatrixExpression {
    using value_type = T;
    static constexpr size_t rows = E::rows;
    static constexpr size_t cols = E::cols;

    // i -> row index, j -> column index
    constexpr value_type at(size_t i, size_t j) const { return static_cast<E const*>(this)->at(i, j); }

    constexpr MatrixRowExpression<T, E> operator[](size_t i) const;
};

}
