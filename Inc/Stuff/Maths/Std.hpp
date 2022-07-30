#include "./BLAS/Concepts.hpp"
#include "./BLAS/MatVec.hpp"

#include <tuple>
#include <utility>

namespace std {

template<::Stf::Concepts::VectorExpression E> struct tuple_size<E> : public integral_constant<size_t, E::vector_size> { };

template<size_t I, ::Stf::Concepts::VectorExpression E> struct tuple_element<I, E> { using type = decay_t<typename E::value_type>; };

}

namespace Stf {

template<size_t I, typename T, size_t N> constexpr T& get(Stf::Vector<T, N>& e) noexcept { return e.data[I]; }

template<size_t I, typename T, size_t N> constexpr T&& get(Stf::Vector<T, N>&& e) noexcept {
    return std::move(e.data[I]);
}

template<size_t I, ::Stf::Concepts::VectorExpression E>
    requires(I < E::vector_size)
constexpr auto get(E const& e) noexcept {
    return e[I];
}

}
