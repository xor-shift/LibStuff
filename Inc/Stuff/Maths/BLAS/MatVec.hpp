#pragma once

#include "./Vector.hpp"
#include "./Matrix.hpp"

namespace Stf::Detail {

/// @ownership
/// This expression object does not own inner expressions
template<typename T, typename E> struct MatrixRowExpression : public VectorExpression<T, MatrixRowExpression<T, E>> {
    using value_type = T;
    static constexpr size_t vector_size = E::cols;

    const size_t i;
    E const& e;

    constexpr value_type operator[](size_t j) const { return e.at(i, j); }
};

template<typename T, typename E>
constexpr MatrixRowExpression<T, E> MatrixExpression<T, E>::operator[](size_t i) const {
    return {
        {},
        i,
        *static_cast<E const*>(this),
    };
}

template<Concepts::MatrixExpression E, bool owning = true>
struct MatrixToVectorExpression
    : public VectorExpression<typename E::value_type, MatrixToVectorExpression<E>>
/*, public MatrixExpression<typename E::value_type, MatrixToVectorExpression<E>>*/ {
    using value_type = typename E::value_type;
    static constexpr size_t vector_size = E::rows * E::cols;
    // static constexpr size_t rows = E::rows;
    // static constexpr size_t cols = E::cols;

    std::conditional_t<owning, E const, E const&> e;

    constexpr value_type operator[](size_t i) const { return e.at(i / E::cols, i % E::cols); }

    // constexpr value_type at(size_t i, size_t j) const { return e.at(i, j); }
};

template<Concepts::VectorExpression E, size_t C, bool owning = true>
struct VectorToMatrixExpression
    : public MatrixExpression<typename E::value_type, VectorToMatrixExpression<E, C>>
/*, public VectorExpression<typename E::value_type, VectorToMatrixExpression<E, C>>*/ {
    using value_type = typename E::value_type;
    static constexpr size_t vector_size = E::vector_size;
    static constexpr size_t rows = vector_size / C;
    static constexpr size_t cols = C;

    static_assert(vector_size % C == 0,
        "vector to matrix adaption requires that the vector can be divided into the specified row size");

    std::conditional_t<owning, E const, E const&> e;

    // constexpr value_type operator[](size_t i) const { return e[i]; }

    constexpr value_type at(size_t i, size_t j) const { return e[i * C + j]; }
};

}


namespace Stf {

template<Concepts::MatrixExpression E> constexpr Detail::MatrixTransposeExpression<E> transpose(E const& e) {
    return { {}, e };
}

template<Concepts::VectorExpression E> constexpr Detail::VectorToMatrixExpression<E, 1> transpose(E const& e) {
    const auto vec_mat = Detail::VectorToMatrixExpression<E, 1> { {}, e };
    return vec_mat;
}

template<Concepts::MatrixExpression E0, Concepts::VectorExpression E1>
constexpr auto operator*(E0 const& mat, E1 const& vec) {
    const auto vec_mat_T = transpose(vec);
    const auto mult = transpose(mat * vec_mat_T);
    const Detail::MatrixToVectorExpression<decltype(mult), true> mult_vec { {}, mult };
    return mult_vec;
}

}
