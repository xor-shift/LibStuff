#pragma once

#include "./Concepts.hpp"
#include "./Util.hpp"

namespace Stf {

template<typename T, size_t R, size_t C> struct Matrix {
    using value_type = T;
    static constexpr size_t rows = R;
    static constexpr size_t cols = C;

    T data[R * C] {};

    constexpr value_type at(size_t i, size_t j) const { return data[i * C + j]; }
};

template<typename T, size_t R, size_t C, typename... Ts>
    requires(R* C == 1 + sizeof...(Ts))
constexpr Matrix<T, R, C> matrix(T v, Ts... vs) {
    return { {}, vector(v, vs...) };
}

template<typename T>
constexpr Matrix<T, 3, 3> rotation_matrix_3d(T yaw, T pitch, T roll) {
    const auto a = yaw;
    const auto b = pitch;
    const auto g = roll;

    const auto sa = std::sin(a);
    const auto sb = std::sin(b);
    const auto sg = std::sin(g);
    const auto ca = std::cos(a);
    const auto cb = std::cos(b);
    const auto cg = std::cos(g);

    return {
        cb*cg, sa*sb*cg - ca*sg, ca*sb*cg + sa*sg,
        cb*sg, sa*sb*sg + ca*cg, ca*sb*sg - sa*cg,
        -sb, sa*cb, ca*cb,
    };
}

template<typename T>
constexpr Matrix<T, 3, 3> rotation_matrix_3d(T yaw, T pitch) {
    const auto a = yaw;
    const auto b = pitch;

    const auto sa = std::sin(a);
    const auto sb = std::sin(b);
    const auto ca = std::cos(a);
    const auto cb = std::cos(b);

    return {
        cb, sa*sb, ca*sb,
        0, ca, -sa,
        -sb, sa*cb, ca*cb,
    };
}

template<Concepts::MatrixExpression E>
requires(E::rows == 3) && (E::cols == 3)
constexpr auto transform_to_homogenous(E const& e) {
    using T = typename E::value_type;
    return Matrix<T, 4, 4> {
        e.at(0, 0), e.at(0, 1), e.at(0, 2), static_cast<T>(0),
        e.at(1, 0), e.at(1, 1), e.at(1, 2), static_cast<T>(0),
        e.at(2, 0), e.at(2, 1), e.at(2, 2), static_cast<T>(0),
        static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1),
    };
}

}

// Expression structs

namespace Stf::Detail {

/// @ownership
/// This expression objects owns inner expressions
template<Concepts::MatrixExpression E> struct MatrixTransposeExpression {
    using value_type = typename E::value_type;
    static constexpr size_t cols = E::rows;
    static constexpr size_t rows = E::cols;

    const E e;

    constexpr value_type at(size_t i, size_t j) const { return e.at(j, i); };
};

/// @ownership
/// This expression owns inner expressions
template<typename Oper, Concepts::MatrixExpression... Es>
    requires VariadicMatrixHelper<Es...>::is_compatible
struct ElementwiseBinaryMatrixExpression {
    using Helper = VariadicMatrixHelper<Es...>;

    using value_type = std::invoke_result_t<Oper, typename Helper::value_type, typename Helper::value_type>;
    static constexpr size_t rows = Helper::rows;
    static constexpr size_t cols = Helper::cols;

    const Oper op;
    const std::tuple<Es...> expressions;

    constexpr value_type at(size_t i, size_t j) const {
        return fold(i * cols + j, expressions, op, [](auto const& expr, size_t idx) { return expr.at(idx / cols, idx % cols); });
    }
};

#define BASIC_BINARY_FACTORY(EXPR_NAME, EXPR_SYM, EXPR_OP_TYPE)                                                                                                \
    template<Concepts::MatrixExpression... Es> using Matrix##EXPR_NAME##Expression = ElementwiseBinaryMatrixExpression<EXPR_OP_TYPE, Es...>;                   \
                                                                                                                                                               \
    template<Concepts::MatrixExpression E, Concepts::MatrixExpression... Es>                                                                                   \
    constexpr Matrix##EXPR_NAME##Expression<Es..., E> operator EXPR_SYM(Matrix##EXPR_NAME##Expression<Es...> e_s, E e) {                                       \
        return { {}, {}, std::tuple_cat(std::move(e_s.expressions), std::tuple<E>(e)) };                                                                       \
    }                                                                                                                                                          \
                                                                                                                                                               \
    template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1> constexpr Matrix##EXPR_NAME##Expression<E0, E1> operator EXPR_SYM(E0 e_0, E1 e_1) { \
        return { {}, {}, { e_0, e_1 } };                                                                                                                       \
    }

BASIC_BINARY_FACTORY(Addition, +, std::plus<>)
BASIC_BINARY_FACTORY(Subtraction, -, std::minus<>)

#undef BASIC_BINARY_FACTORY

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
    requires(E0::cols == E1::rows)
struct MatrixMultiplicationExpression {
    using value_type = typename E0::value_type;
    static constexpr size_t cols = E1::cols;
    static constexpr size_t rows = E0::rows;

    const E0 e_0;
    const E1 e_1;

    constexpr value_type at(size_t i, size_t j) const {
        // i -> the col to scan on e_1
        // j -> the row to scan on e_0

        value_type sum = 0;
        for (size_t k = 0; k < E1::rows; k++) {
            const auto lhs = e_0.at(i, k);
            const auto rhs = static_cast<value_type>(e_1.at(k, j));
            sum += lhs * rhs;
        }

        return sum;
    }
};

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr MatrixMultiplicationExpression<E0, E1> operator*(E0 const& e_0, E1 const& e_1) {
    return { {}, e_0, e_1 };
}

}
