#pragma once

#include <concepts>
#include <cstddef>

#include <Stuff/Intro/SAgg.hpp>

template<typename T> struct Introspector;

namespace Stf::Detail {

/// Do not confuse with ADL-get, this is for introspectors
template<typename T, size_t N, typename Res>
concept GettableAt = //
  requires(T& l, T&& r, T const& cl, T const&& cr) {
      { T::template get<N>(l) } -> std::same_as<Res&>;
      { T::template get<N>(r) } -> std::same_as<Res&&>;
      { T::template get<N>(cl) } -> std::same_as<Res const&>;
      { T::template get<N>(cr) } -> std::same_as<Res const&&>;
  };

template<size_t N, typename Res, typename T> struct GettableHelper;

template<typename Res, GettableAt<0, Res> T> struct GettableHelper<0, Res, T> { };

template<size_t N, typename Res, GettableAt<N, Res> T>
    requires(requires { typename GettableHelper<N - 1, Res, T>; })
struct GettableHelper<N, Res, T> { };

template<size_t N, typename Res, typename T> struct MultiGettableHelper;

template<typename Res, GettableAt<0, Res> T> struct MultiGettableHelper<0, ABunchOfTypes<Res>, T> { };

template<size_t N, typename Res, typename... Us, GettableAt<N, Res> T>
    requires(requires { typename MultiGettableHelper<N - 1, ABunchOfTypes<Res, Us...>, T>; })
struct MultiGettableHelper<N, ABunchOfTypes<Res, Us...>, T> { };

}

namespace Stf {

/* Introspector categories:
 * s -> static or single or is&ns
 * d -> dynamic or id&nd
 * m -> multiple
 *
 *
 * i(s|d) -> integral indices
 * n(s|d) -> named indices
 * i -> is&id
 * n -> ns&nd
 * all -> i&n
 *
 * Notes:
 * introspectors with named static indices must have enumerable indices
 * a dynamic index on a multi-typed introspector should return a variant
 *
 *               |length|type|indexing| example(s)
 *               +------+----+--------+
 * List          |d     |s   |id      | std::vector<T>, std::span<T>
 * Array         |s     |s   |i       | std::array<T, n>, T[N]
 * Tuple         |s     |m   |is      | std::tuple, SAgg
 * NamedTuple    |s     |m   |s       | struct
 * DynTuple      |d     |m   |id      | vector<variant<...>> i.e. JSON array
 * Object        |d     |m   |d       | NamedDynTuple, unordered_map<string_view, variant<...>> i.e. JSON object
 */

template<typename T>
concept ListIntrospector = //
  requires(typename T::type const& v, size_t i) {
      typename T::type;
      typename T::member_type;
      { T::size(std::declval<typename T::type&>()) } -> std::convertible_to<size_t>;
      { T::index(std::declval<typename T::type&>(), i) } -> std::convertible_to<typename T::member_type&>;
      { T::index(std::declval<typename T::type const&>(), i) } -> std::convertible_to<typename T::member_type const&>;
      { T::index(std::declval<typename T::type&&>(), i) } -> std::convertible_to<typename T::member_type&&>;
      { T::index(std::declval<typename T::type const&&>(), i) } -> std::convertible_to<typename T::member_type const&&>;
  };

template<typename T>
concept ArrayIntrospector = //
  ListIntrospector<T> &&    //
  requires(size_t i) {
      { T::size() } -> std::convertible_to<size_t>;
      typename Detail::GettableHelper<T::size() - 1, typename T::type, T>;
  };

template<typename T>
concept TupleIntrospector = //
  requires() {
      typename T::type;
      typename T::types;

      { T::size() } -> std::convertible_to<size_t>;

      typename Detail::MultiGettableHelper<T::size() - 1, typename T::types, T>;
  };

template<typename T, typename Key = std::string_view>
concept ObjectIntrospector = //
  requires(typename T::type const& cl, typename T::type& l) {
      typename T::type;
      typename T::key_type;
      typename T::member_type;

      { T::size(cl) } -> std::convertible_to<size_t>;

      { T::at(cl, std::declval<typename T::key_type const&>()) } -> std::convertible_to<typename T::member_type const&>;
      { T::at(l, std::declval<typename T::key_type&>()) } -> std::convertible_to<typename T::member_type&>;

      T::iterate(cl, [](typename T::key_type, typename T::member_type const&) {});
      T::iterate(l, [](typename T::key_type, typename T::member_type&) {});
  };

namespace Detail {

template<typename T>
concept ADLIntrospectable = requires() { _libstf_adl_introspector(std::declval<T>()); };

template<typename T>
concept GlobalIntrospectable = requires() { typename ::Introspector<T>::type; };

template<typename T, typename SFINAE = void> struct ADLIntroHelper;

template<ADLIntrospectable T> struct ADLIntroHelper<T, std::enable_if_t<ADLIntrospectable<T>>> {
    using type = decltype(_libstf_adl_introspector(std::declval<T>()));
};

template<GlobalIntrospectable T>
struct ADLIntroHelper<T, std::enable_if_t<GlobalIntrospectable<T> && !ADLIntrospectable<T>>> {
    using type = ::Introspector<T>;
};

template<SAgg T>
struct ADLIntroHelper<T, std::enable_if_t<SAgg<T> && !GlobalIntrospectable<T> && !ADLIntrospectable<T>>> {
    using type = ::Stf::SAggIntrospector<T>;
};

}

template<typename T> using introspector_type = typename Detail::ADLIntroHelper<T>::type;

/// NOTE: n of T (T[n]) types are not possible to introspect outside of the global namespace
/// @example
/// @code

/// @endcode
#define STF_MAKE_ADL_INTROSPECTOR(...)                                                           \
    inline Introspector<__VA_ARGS__> _libstf_adl_introspector(__VA_ARGS__&&) {                   \
        static_assert(::Stf::False<__VA_ARGS__>::value, "do not call _libstf_adl_introspector"); \
    }

}

/*#include <Stuff/Intro/HelperTypes.hpp>

namespace Foo {
    struct Bar { // SAgg of Bar is false, no generated tuple-like Introspector is available
        int a;
        int b[5];
    };

    template<typename T> struct Introspector;
    template<> struct Introspector<Bar> { // a static object/named tuple (think JSON) introspector
        using type = Bar;
        using key_type = std::string_view;
        inline
        using member_types = ::Stf::ABunchOfTypes<int, int[5]>;

        template<Stf::ArrayString Key>
        static auto const& at(type const& v) {
            if constexpr (Key == "a") return v.a;
            if constexpr (Key == "b") return v.b;
        }

        static auto const& at(type const& v, key_type key) {
        }
    };
}*/
