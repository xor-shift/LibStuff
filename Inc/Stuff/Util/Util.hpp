#pragma once

#include <cstddef>
#include <functional>

#include "./Conv.hpp"
#include "./CoroCompat.hpp"
#include "./DummyIterator.hpp"
#include "./Error.hpp"
#include "./Scope.hpp"
#include "./SpinLock.hpp"

namespace Stf {

// clang-format off

namespace Detail {

template<size_t I, typename T>
constexpr auto getter(T const& v) -> decltype(v.template get<I>()) { return v.template get<I>(); }

template<size_t I, typename T>
constexpr auto getter(T& v) -> decltype(v.template get<I>()) { return v.template get<I>(); }

template<size_t I, typename T>
constexpr auto getter(T const&& v) -> decltype(v.template get<I>()) { return std::move(v.template get<I>()); }

template<size_t I, typename T>
constexpr auto getter(T&& v) -> decltype(v.template get<I>()) { return std::move(v.template get<I>()); }

template<size_t I, typename T>
constexpr auto getter(T const& v) -> decltype(get<I>(v)) { return get<I>(v); }

template<size_t I, typename T>
constexpr auto getter(T& v) -> decltype(get<I>(v)) { return get<I>(v); }

template<size_t I, typename T>
constexpr auto getter(T const&& v) -> decltype(get<I>(v)) { return std::move(get<I>(std::forward(v))); }

template<size_t I, typename T>
constexpr auto getter(T&& v) -> decltype(get<I>(v)) { return std::move(get<I>(std::forward(v))); }

}

// clang-format on

/// Calls `get<I>` as described in [dcl.struct.bind]\n
/// i.e. get is first search on the scope of T, if a member function cannot be
/// found to call `v.get<I>()`, ADL is performed to perform the call
/// `get<I, T>(v)`
/// \tparam I
/// \tparam T
/// \param arg
/// \return
template<size_t I, typename T>
constexpr auto get(T&& arg) -> decltype(auto) { return Detail::getter<I>(arg); }

template<typename... Funcs>
struct MultiVisitor : public Funcs...{
    using Funcs::operator()...;
};

template<typename... Funcs>
MultiVisitor(Funcs&&...) -> MultiVisitor<Funcs...>;


}
