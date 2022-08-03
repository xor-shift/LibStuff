#pragma once

#include "./Matrix.hpp"
#include "./Vector.hpp"

namespace Stf::Detail {

/// @ownership
/// This expression object does not own inner expressions
template<typename T, typename E> struct MatrixRowExpression {
    using value_type = T;
    static constexpr size_t vector_size = E::cols;

    const size_t i;
    E const& e;

    constexpr value_type operator[](size_t j) const { return e.at(i, j); }
};

template<Concepts::MatrixExpression E, bool owning = true> struct MatrixToVectorExpression {
    using value_type = typename E::value_type;
    static constexpr size_t vector_size = E::rows * E::cols;
    // static constexpr size_t rows = E::rows;
    // static constexpr size_t cols = E::cols;

    std::conditional_t<owning, E const, E const&> e;

    constexpr value_type operator[](size_t i) const { return e.at(i / E::cols, i % E::cols); }

    // constexpr value_type at(size_t i, size_t j) const { return e.at(i, j); }
};

template<Concepts::VectorExpression E, size_t C, bool owning = true> struct VectorToMatrixExpression {
    using value_type = typename E::value_type;
    static constexpr size_t vector_size = E::vector_size;
    static constexpr size_t rows = vector_size / C;
    static constexpr size_t cols = C;

    static_assert(vector_size % C == 0, "vector to matrix adaption requires that the vector can be divided into the specified row size");

    std::conditional_t<owning, E const, E const&> e;

    // constexpr value_type operator[](size_t i) const { return e[i]; }

    constexpr value_type at(size_t i, size_t j) const { return e[i * C + j]; }
};

}

namespace Stf {

template<Concepts::MatrixExpression E> constexpr Detail::MatrixTransposeExpression<E> transpose(E const& e) { return { {}, e }; }

template<Concepts::VectorExpression E> constexpr Detail::VectorToMatrixExpression<E, 1> transpose(E const& e) {
    const auto vec_mat = Detail::VectorToMatrixExpression<E, 1> { {}, e };
    return vec_mat;
}

template<Concepts::MatrixExpression E0, Concepts::VectorExpression E1> constexpr auto operator*(E0 const& mat, E1 const& vec) {
    const auto vec_mat_T = transpose(vec);
    const auto mult = transpose(mat * vec_mat_T);
    const Detail::MatrixToVectorExpression<decltype(mult), true> mult_vec { {}, mult };
    return mult_vec;
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1, Concepts::VectorExpression E2 = Vector<typename E1::value_type, E1::vector_size>>
constexpr Matrix<typename E0::value_type, 4, 4> matrix_look_at(
    E0 const& eye, E1 const& at, E2 const& upp = vector<typename E1::value_type>(0, 1, 0), bool right_handed = false) {
    using T = typename E0::value_type;

    const auto view = normalized(right_handed ? eye - at : at - eye);
    const auto u_cross_v = cross(upp, view);
    const auto right = normalized(u_cross_v);
    const auto up = cross(view, right);

    // clang-format off
    return {
        right[0], up[0], view[0], static_cast<T>(0),
        right[1], up[1], view[1], static_cast<T>(0),
        right[2], up[2], view[2], static_cast<T>(0),
        -dot(right, eye), -dot(up, eye), -dot(view, eye), static_cast<T>(1),
    };
    // clang-format on
}

}
