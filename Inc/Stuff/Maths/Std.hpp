#include "./BLAS/Concepts.hpp"
#include "./BLAS/MatVec.hpp"

#include <tuple>

namespace std {

template<size_t I, typename T, size_t N> constexpr T const& get(Stf::Vector<T, N> const& e) noexcept { return e.data[I]; }

template<size_t I, typename T, size_t N> constexpr T& get(Stf::Vector<T, N>& e) noexcept { return e.data[I]; }

template<size_t I, typename T, size_t N> constexpr T&& get(Stf::Vector<T, N>&& e) noexcept {
    return std::move(e.data[I]);
}

template<::Stf::Concepts::VectorExpression E>
struct tuple_size<E> : public integral_constant<size_t, E::vector_size> { };

template<size_t I, ::Stf::Concepts::VectorExpression E> struct tuple_element<I, E> {
    using type = typename E::value_type;
};

template<size_t I, ::Stf::Concepts::VectorExpression E> constexpr typename E::value_type get(E const& e) noexcept {
    return e[I];
}

template<::Stf::Concepts::MatrixExpression E>
struct tuple_size<E> : public integral_constant<size_t, E::rows * E::cols> { };

template<size_t I, ::Stf::Concepts::MatrixExpression E> struct tuple_element<I, E> {
    using value_type = typename E::value_type;
};

template<size_t I, ::Stf::Concepts::MatrixExpression E> constexpr typename E::value_type get(E const& e) noexcept {
    return ::Stf::Detail::MatrixToVectorExpression<E, false> { e }[I];
}

}

namespace Stf::Refl {

template<typename T>
struct is_refl;

template<Concepts::VectorExpression E> struct is_refl<E> : public std::true_type { };

}
