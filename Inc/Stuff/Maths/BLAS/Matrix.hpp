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
    constexpr value_type& at(size_t i, size_t j) { return data[i * C + j]; }

    static constexpr Matrix rotation(T yaw, T pitch, T roll)
        requires(R == 3) && (C == 3)
    {
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
            cb * cg,
            sa * sb * cg - ca * sg,
            ca * sb * cg + sa * sg,
            cb * sg,
            sa * sb * sg + ca * cg,
            ca * sb * sg - sa * cg,
            -sb,
            sa * cb,
            ca * cb,
        };
    }

    static constexpr Matrix rotation(T yaw, T pitch, T roll)
        requires(R == 4) && (C == 4)
    {
        const auto base = Matrix<T, 3, 3>::rotation(yaw, pitch, roll);

        return {
            base.at(0, 0), base.at(0, 1), base.at(0, 2), 0, //
            base.at(1, 0), base.at(1, 1), base.at(1, 2), 0, //
            base.at(2, 0), base.at(2, 1), base.at(2, 2), 0, //
            0, 0, 0, 1                                      //
        };
    }

    static constexpr Matrix rotation(T yaw, T pitch)
        requires(R == 3) && (C == 3)
    {
        const auto a = yaw;
        const auto b = pitch;

        const auto sa = std::sin(a);
        const auto sb = std::sin(b);
        const auto ca = std::cos(a);
        const auto cb = std::cos(b);

        return {
            cb, sa * sb, ca * sb, //
            0, ca, -sa,           //
            -sb, sa * cb, ca * cb //
        };
    }

    static constexpr Matrix rotation(T yaw, T pitch)
        requires(R == 4) && (C == 4)
    {
        const auto base = Matrix<T, 3, 3>::rotation(yaw, pitch);

        return {
            base.at(0, 0), base.at(0, 1), base.at(0, 2), 0, //
            base.at(1, 0), base.at(1, 1), base.at(1, 2), 0, //
            base.at(2, 0), base.at(2, 1), base.at(2, 2), 0, //
            0, 0, 0, 1                                      //
        };
    }

    static constexpr Matrix ortographic(T left, T right, T bottom, T top, T near, T far)
        requires(R == 4) && (C == 4)
    {
        const auto two = static_cast<T>(2);
        const auto zero = static_cast<T>(0);
        return {
            two / (right - left), zero, zero, -(right + left) / (right - left), //
            zero, two / (top - bottom), zero, -(top + bottom) / (top - bottom), //
            zero, zero, -two / (far - near), -(far + near) / (far - near),      //
            zero, zero, zero, static_cast<T>(1)                                 //
        };
    }

    static constexpr Matrix look_at(Vector<T, 3> from, Vector<T, 3> at)
        requires(R == 4) && (C == 4)
    { }

    static constexpr Matrix identity()
        requires(R == C)
    {
        Matrix mat {};
        for (auto i = 0uz; i < R; i++)
            mat.at(i, i) = 1;
        return mat;
    }
};

template<typename T, size_t R, size_t C, typename... Ts>
    requires(R* C == 1 + sizeof...(Ts))
constexpr Matrix<T, R, C> matrix(T v, Ts... vs) {
    return { {}, vector(v, vs...) };
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
