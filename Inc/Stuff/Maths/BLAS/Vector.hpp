#pragma once

#include "./Concepts.hpp"
#include "./Util.hpp"

#include <Stuff/Maths/Scalar.hpp>

namespace Stf {

template<typename T, size_t N> struct Vector {
    T data[N];

    using value_type = T;
    constexpr static size_t vector_size = N;

    constexpr T operator[](size_t i) const { return data[i]; }

    constexpr T& operator[](size_t i) { return data[i]; }
};

template<typename T, size_t N> struct RefVector {
    T* data[N];

    using value_type = T;
    constexpr static size_t vector_size = N;

    constexpr T const& operator[](size_t i) const { return *data[i]; }
};

template<typename T, typename... Ts> constexpr Vector<T, 1 + sizeof...(Ts)> vector(const T v, Ts... vs) { return { static_cast<T>(v), static_cast<T>(vs)... }; }

template<Concepts::VectorExpression E> constexpr auto vector(E const& e) {
    Vector<typename E::value_type, E::vector_size> ret {};

    for (size_t i = 0; i < E::vector_size; i++)
        ret[i] = e[i];

    return ret;
}

}

// Expression structs
namespace Stf::Detail {

template<typename Oper, Concepts::VectorExpression... Es>
    requires VariadicVectorHelper<Es...>::is_compatible
struct ElementwiseBinaryVectorOperation {
    using Helper = VariadicVectorHelper<Es...>;
    using value_type = std::invoke_result_t<Oper, typename Helper::value_type, typename Helper::value_type>;
    static constexpr size_t vector_size = Helper::vector_size;

    const Oper op;
    const std::tuple<Es...> expressions;

    constexpr value_type operator[](size_t i) const {
        return fold(i, expressions, op, [](auto const& expr, size_t idx) { return expr[idx]; });
    }
};

template<Concepts::VectorExpression E, typename Op> struct VectorMapExpression {
    using value_type = std::invoke_result_t<Op, typename E::value_type>;
    static constexpr size_t vector_size = E::vector_size;

    const E e;
    const Op op;

    constexpr value_type operator[](size_t i) const { return std::invoke(op, e[i]); }
};

// TODO: remove this class and replace with ElementwiseUnaryVectorOperation
template<Concepts::VectorExpression E, typename Scalar, typename Op>
    requires std::convertible_to<Scalar, typename E::value_type>
struct VectorScalarExpression {
    using value_type = typename E::value_type;
    static constexpr size_t vector_size = E::vector_size;

    const E e;
    const Scalar s;
    const Op op;

    constexpr value_type operator[](size_t i) const { return std::invoke(op, e[i], static_cast<typename E::value_type>(s)); }
};

struct RemFNObject {
    template<std::floating_point T> constexpr T operator()(T lhs, T rhs) const { return std::fmod(lhs, rhs); }
    template<std::integral T> constexpr T operator()(T lhs, T rhs) const { return lhs % rhs; }
};

template<Concepts::VectorExpression E> using VectorScalarAdditionExpression = VectorScalarExpression<E, typename E::value_type, std::plus<>>;
template<Concepts::VectorExpression E> using VectorScalarMultiplicationExpression = VectorScalarExpression<E, typename E::value_type, std::multiplies<>>;
template<Concepts::VectorExpression E> using VectorScalarSubtractionExpression = VectorScalarExpression<E, typename E::value_type, std::minus<>>;
template<Concepts::VectorExpression E> using VectorScalarDivisionExpression = VectorScalarExpression<E, typename E::value_type, std::divides<>>;
template<Concepts::VectorExpression E> using VectorScalarRemainderExpression = VectorScalarExpression<E, typename E::value_type, RemFNObject>;

template<Concepts::VectorExpression... Es> using VectorAdditionExpression = ElementwiseBinaryVectorOperation<std::plus<>, Es...>;
template<Concepts::VectorExpression... Es> using VectorMultiplicationExpression = ElementwiseBinaryVectorOperation<std::multiplies<>, Es...>;
template<Concepts::VectorExpression... Es> using VectorSubtractionExpression = ElementwiseBinaryVectorOperation<std::minus<>, Es...>;
template<Concepts::VectorExpression... Es> using VectorDivisionExpression = ElementwiseBinaryVectorOperation<std::divides<>, Es...>;

}

namespace Stf {

#define BASIC_SCALAR_FACTORY(EXPR_NAME, EXPR_SYM)                                                                                         \
    template<Concepts::VectorExpression E, typename T>                                                                                    \
        requires std::convertible_to<T, typename E::value_type>                                                                           \
    constexpr Detail::VectorScalar##EXPR_NAME##Expression<E> operator EXPR_SYM(E const& e, T v) {                                         \
        const auto v_conv = static_cast<typename E::value_type>(v);                                                                       \
        return { e, v_conv, {} };                                                                                                         \
    }                                                                                                                                     \
                                                                                                                                          \
    template<Concepts::VectorExpression E, typename T>                                                                                    \
        requires std::convertible_to<T, typename E::value_type>                                                                           \
    constexpr Detail::VectorScalar##EXPR_NAME##Expression<E> operator EXPR_SYM(Detail::VectorScalar##EXPR_NAME##Expression<E>&& e, T v) { \
        return { e.e, e.s + static_cast<typename T::value_Type>(v), {} };                                                                 \
    }

#define BASIC_BINARY_FACTORY(EXPR_NAME, EXPR_SYM, EXPR_OP_TYPE)                                                                                 \
                                                                                                                                                \
    template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>                                                                      \
    constexpr Detail::Vector##EXPR_NAME##Expression<E0, E1> operator EXPR_SYM(E0 const& e_0, E1 const& e_1) {                                   \
        return { {}, { e_0, e_1 } };                                                                                                            \
    }                                                                                                                                           \
    template<Concepts::VectorExpression E, Concepts::VectorExpression... Es>                                                                    \
    constexpr Detail::Vector##EXPR_NAME##Expression<Es..., E> operator EXPR_SYM(Detail::Vector##EXPR_NAME##Expression<Es...> e_s, E const& e) { \
        return { {}, std::tuple_cat(std::move(e_s.expressions), std::tuple<E>(e)) };                                                            \
    }

BASIC_SCALAR_FACTORY(Addition, +)
BASIC_SCALAR_FACTORY(Subtraction, -)
BASIC_SCALAR_FACTORY(Multiplication, *)
BASIC_SCALAR_FACTORY(Division, /)
BASIC_SCALAR_FACTORY(Remainder, %)

BASIC_BINARY_FACTORY(Addition, +, std::plus<>)
BASIC_BINARY_FACTORY(Subtraction, -, std::minus<>)
BASIC_BINARY_FACTORY(Multiplication, *, std::multiplies<>)
BASIC_BINARY_FACTORY(Division, /, std::divides<>)

#undef BASIC_SCALAR_FACTORY
#undef BASIC_BINARY_FACTORY

template<typename Op, Concepts::VectorExpression E> constexpr auto fold(E const& e, Op op = {}) {
    auto v = e[0];

    if constexpr (E::vector_size == 1)
        return v;

    for (size_t i = 1; i < E::vector_size; i++)
        v = op(v, e[i]);

    return v;
}

template<typename Op, Concepts::VectorExpression E> constexpr Detail::VectorMapExpression<E, Op> map(E const& e, Op op = {}) { return { {}, e, op }; }

template<Concepts::VectorExpression E> constexpr auto magnitude(E const& e) { return fold(e, std::plus<> {}); }

template<Concepts::VectorExpression E> constexpr auto normalized(E const& e) {
    auto decayed = vector(e);

    auto mag = magnitude(e);
    for (auto& v : decayed.data)
        v /= mag;

    return decayed;
}

template<Concepts::VectorExpression E> constexpr auto abs(E const& e) {
    auto op = [](typename E::value_type v) { return Stf::abs(v); };
    return Detail::VectorMapExpression<E, decltype(op)> { {}, e, op };
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
    requires(E0::vector_size == E1::vector_size)
constexpr auto dot(E0 const& e_0, E1 const& e_1) {
    return fold(e_0 * e_1, std::plus<> {});
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
    requires(E0::vector_size == 3) && (E1::vector_size == 3)
constexpr Vector<typename E0::value_type, 3> cross(E0 const& e_0, E1 const& e_1) {
    return {
        e_0[1] * e_1[2] - e_0[2] * e_1[1],
        e_0[2] * e_1[0] - e_0[0] * e_1[2],
        e_0[0] * e_1[1] - e_0[1] * e_1[0],
    };
}

}
