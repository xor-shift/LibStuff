#pragma once

#include <concepts>
#include <cstddef>

#include <Stuff/Intro/SAgg.hpp>

template<typename T> struct Introspector;

namespace Stf::Detail {

/// Do not confuse with ADL get, this is for introspectors
template<typename T, auto Key, typename Res>
concept GettableAtWithType = //
  requires(typename T::type& l, typename T::type&& r, typename T::type const& cl, typename T::type const&& cr) {
      { T::template get<Key>(l) } -> std::same_as<Res&>;
      { T::template get<Key>(r) } -> std::same_as<Res&&>;
      { T::template get<Key>(cl) } -> std::same_as<Res const&>;
      { T::template get<Key>(cr) } -> std::same_as<Res const&&>;
  };

template<size_t N, typename Res, typename T> struct ListGettableHelper;

template<typename Res, GettableAtWithType<0, Res> T> struct ListGettableHelper<0, Res, T> { };

template<size_t N, typename Res, GettableAtWithType<N, Res> T>
    requires(requires { typename ListGettableHelper<N - 1, Res, T>; })
struct ListGettableHelper<N, Res, T> { };

template<typename T, auto Key>
concept GettableAt = //
  requires(typename T::type& l, typename T::type&& r, typename T::type const& cl, typename T::type const&& cr) {
      T::template get<Key>(l);
      T::template get<Key>(r);
      T::template get<Key>(cl);
      T::template get<Key>(cr);
  };

template<size_t N, typename T> struct TupleGettableHelper;

template<GettableAt<0> T> struct TupleGettableHelper<0, T> { };

template<size_t N, GettableAt<N> T>
    requires(requires { typename TupleGettableHelper<N - 1, T>; })
struct TupleGettableHelper<N, T> { };

template<auto> struct RequireConstant;

}

namespace Stf {

/*
 * TODO: relax the rvalue reference requirements? (this causes issue with "built" struct introspectors containing
 * transformed members)
 *
 * all Introspector<T> must satisfy:
 *  a typename ::type
 *   -> U will be `typename T::type` from now on. additionally, the following declarations are to be assumed:
 *      U& l, U const& cl, U&& r, U const&& cr
 *  a non-constexpr ::size(cl), which quantifies some "size"
 *
 * tuples must additionally implement:
 *  ::size() with the added constraint of this function being a constant expression
 *  ::get<I>(l), ::get<I>(cl), ::get<I>(r), ::get<I>(cr) must all be callable with I up to but not including ::size()
 *
 * lists must additionally implement:
 *  a typename ::member_type
 *  ::index(l, I), ::index(cl, I), ::index(r, I), ::index(cr, I)
 *   -> all of these should return *something* convertible to ::member_type
 *
 * arrays must additionally implement:
 *  - everything from lists
 *  - everything from tuples
 *
 * structs must additionally implement:
 *  - everything from tuples
 *  a value ::keys which is a Stf::ABunchOfValues<...> where all values are Stf::ArrayString
 *  ::key_name<I>() which should return a std::string_view for all I up to but not including ::size()
 *  ::get<Name>(l), ::get<Name>(cl), ::get<Name>(r), ::get<Name>(cr) should all be callable with all return values from
 *    ::key_name<I>() with I constrained as above. The return types mustn't differ between the related Name and I
 *
 * objects must additionally implement:
 *  ::at(l, name), ::at(cl, name) where name is some std::string_view. These might throw. TODO: any better approach?
 *  ::iterate(l, fn), ::iterate(cl, fn) TODO: describe
 *
 * maps must additionally implement:
 *  - everything from objects
 *  a typename ::member_type
 *   -> ::at calls must be convertible to this type, ::iterate calls must call fn with this type
 *
 */

template<typename T>
concept BaseIntrospector = //
  requires(typename T::type const& cl) {
      typename T::type;

      { T::size(cl) } -> std::convertible_to<size_t>;
  };

template<typename T>
concept TupleIntrospector = //
  BaseIntrospector<T> &&    //
  requires() {
      { T::size() } -> std::convertible_to<size_t>;
      typename Detail::RequireConstant<T::size()>;

      typename Detail::TupleGettableHelper<T::size() - 1, T>;
  };

template<typename T>
concept ListIntrospector = //
  BaseIntrospector<T> &&   //
  requires(typename T::type const& v, size_t i) {
      typename T::member_type;

      { T::index(std::declval<typename T::type&>(), i) } -> std::convertible_to<typename T::member_type&>;
      { T::index(std::declval<typename T::type const&>(), i) } -> std::convertible_to<typename T::member_type const&>;
      { T::index(std::declval<typename T::type&&>(), i) } -> std::convertible_to<typename T::member_type&&>;
      { T::index(std::declval<typename T::type const&&>(), i) } -> std::convertible_to<typename T::member_type const&&>;
  };

template<typename T>
concept ArrayIntrospector = ListIntrospector<T> && TupleIntrospector<T>;

template<typename T>
concept StructIntrospector = //
  TupleIntrospector<T> &&    //
  requires() {
      typename T::keys;

      // TODO: add something like TupleGettableHelper but for a group of keys and expected types
      // TODO: validate that all of T::keys are Stf::ArrayString
  };

template<typename T>
concept ObjectIntrospector = //
  BaseIntrospector<T> &&     //
  requires(typename T::type const& cl, typename T::type& l) {
      T::at(cl, std::declval<std::string_view>());
      T::at(l, std::declval<std::string_view>());

      T::iterate(cl, [](std::string_view, auto const&) {});
      T::iterate(l, [](std::string_view, auto&) {});
  };

template<typename T>
concept MapIntrospector =  //
  ObjectIntrospector<T> && //
  requires() { typename T::member_type; };

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
