#include "./BLAS/Concepts.hpp"
#include "./BLAS/MatVec.hpp"

#include <tuple>
#include <utility>

namespace std {

template<::Stf::Concepts::VectorExpression E>
struct tuple_size<E> : public integral_constant<size_t, E::vector_size> { };

template<size_t I, ::Stf::Concepts::VectorExpression E> struct tuple_element<I, E> {
    using type = std::decay_t<typename E::value_type>;
};

//template<size_t I, typename T, size_t N> constexpr T const& get(Stf::Vector<T, N> const& e) noexcept { return e.data[I]; }

/*template<::Stf::Concepts::MatrixExpression E>
struct tuple_size<E> : public integral_constant<size_t, E::rows * E::cols> { };

template<size_t I, ::Stf::Concepts::MatrixExpression E> struct tuple_element<I, E> {
    using value_type = std::decay_t<typename E::value_type>;
};

template<size_t I, ::Stf::Concepts::MatrixExpression E> constexpr typename tuple_element<I, E>::type get(E const& e) noexcept {
    return ::Stf::Detail::MatrixToVectorExpression<E, false> { e }[I];
}*/

}

namespace Stf {

template<size_t I, typename T, size_t N> constexpr T& get(Stf::Vector<T, N>& e) noexcept { return e.data[I]; }

template<size_t I, typename T, size_t N> constexpr T&& get(Stf::Vector<T, N>&& e) noexcept {
    return std::move(e.data[I]);
}

template<size_t I, ::Stf::Concepts::VectorExpression E> constexpr typename std::tuple_element<I, E>::type get(E const& e) noexcept {
    return e[I];
}

}

namespace Stf::Refl {

template<typename T>
struct is_refl;

template<Concepts::VectorExpression E> struct is_refl<E> : public std::true_type { };

}
