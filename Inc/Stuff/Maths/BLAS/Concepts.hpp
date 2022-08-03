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
