#pragma once

#include <cstddef>
#include <functional>

#include <Stuff/Util/Util.hpp>

#define MEMREFL_BEGIN(_name, _count)                                                                                             \
    struct MemReflBaseHelper {                                                                                                       \
        using parent_type = _name;                                                                                                   \
                                                                                                                                     \
        static constexpr size_t member_count = 5;                                                                                    \
        static constexpr size_t ct_base = __COUNTER__;                                                                               \
    };                                                                                                                               \
    template<size_t R_I, typename = void>                                                                                            \
        requires(R_I < MemReflBaseHelper::member_count)                                                                              \
    struct MemReflHelper;                                                                                                            \
                                                                                                                                     \
    template<size_t R_I>                                                                                                             \
        requires(R_I < MemReflBaseHelper::member_count)                                                                              \
    using refl_getter_type = typename MemReflHelper<R_I>::type;                                                                      \
                                                                                                                                     \
    template<size_t R_I> constexpr refl_getter_type<R_I> const& get() const& { return MemReflHelper<R_I>::get(*this); }              \
    template<size_t R_I> constexpr refl_getter_type<R_I>& get()& { return MemReflHelper<R_I>::get(*this); }                          \
    template<size_t R_I> constexpr refl_getter_type<R_I> const&& get() const&& { return std::move(MemReflHelper<R_I>::get(*this)); } \
    template<size_t R_I> constexpr refl_getter_type<R_I>&& get()&& { return std::move(MemReflHelper<R_I>::get(*this)); }

#define MEMREFL_MEMBER(name)                                                                                                                          \
    template<size_t R_I> struct MemReflHelper<R_I, std::enable_if_t<R_I == __COUNTER__ - MemReflBaseHelper::ct_base - 1>> {                               \
        using type = decltype(MemReflBaseHelper::parent_type::name);                                                                                      \
                                                                                                                                                          \
        static constexpr type const& get(typename MemReflBaseHelper::parent_type const& p) { return p.name; }                                             \
        static constexpr type const&& get(typename MemReflBaseHelper::parent_type const&& p) { return std::move(p.name); }                                \
        template<typename R_T = type> static constexpr std::enable_if_t<!std::is_const_v<R_T>, type&> get(typename MemReflBaseHelper::parent_type& p) {   \
            return p.name;                                                                                                                                \
        }                                                                                                                                                 \
        template<typename R_T = type> static constexpr std::enable_if_t<!std::is_const_v<R_T>, type&&> get(typename MemReflBaseHelper::parent_type&& p) { \
            return std::move(p.name);                                                                                                                     \
        }                                                                                                                                                 \
    }

#define MEMREFL_DECL_MEMBER(name) \
    name = {};                    \
    MEMREFL_MEMBER(name)

#define EXTREFL_BEGIN(_name)                                                                                                                     \
    template<size_t I, typename T> struct ReflGetHelper;                                                                                             \
    template<typename T> struct ReflMainHelper;                                                                                                      \
                                                                                                                                                     \
    template<> struct ReflMainHelper<_name> { inline static constexpr size_t ct_base = __COUNTER__; };                                               \
                                                                                                                                                     \
    template<size_t I> constexpr auto get(_name const& v)->typename ReflGetHelper<I, _name>::type const& { return ReflGetHelper<I, _name>::get(v); } \
    template<size_t I> constexpr auto get(_name const&& v)->typename ReflGetHelper<I, _name>::type const&& {                                         \
        return std::forward<typename ReflGetHelper<I, _name>::type>(ReflGetHelper<I, _name>::get(v));                                                \
    }                                                                                                                                                \
    template<size_t I> constexpr auto get(_name& v)->typename ReflGetHelper<I, _name>::type& { return ReflGetHelper<I, _name>::get(v); }             \
    template<size_t I> constexpr auto get(_name&& v)->typename ReflGetHelper<I, _name>::type&& {                                                     \
        return std::forward<const typename ReflGetHelper<I, _name>::type>(ReflGetHelper<I, _name>::get(v));                                          \
    }

#define EXTREFL_MEMBER(_class_name, _member_name)                                                      \
    template<> struct ReflGetHelper<__COUNTER__ - ReflMainHelper<_class_name>::ct_base - 1, _class_name> { \
        using type = decltype(_class_name::_member_name);                                                  \
                                                                                                           \
        static constexpr type const& get(_class_name const& p) { return p._member_name; }                  \
        static constexpr type const&& get(_class_name const&& p) { return std::move(p._member_name); }     \
        static constexpr type& get(_class_name& p) { return p._member_name; }                              \
        static constexpr type&& get(_class_name&& p) { return std::move(p._member_name); }                 \
    };

namespace Stf::Refl {

namespace Detail {

// clang-format off

template<size_t I, typename T> constexpr auto getter(T const& v) -> decltype(v.template get<I>()) { return v.template get<I>(); }
template<size_t I, typename T> constexpr auto getter(T& v) -> decltype(v.template get<I>()) { return v.template get<I>(); }
template<size_t I, typename T> constexpr auto getter(T const&& v) -> decltype(v.template get<I>()) { return std::move(v.template get<I>()); }
template<size_t I, typename T> constexpr auto getter(T&& v) -> decltype(v.template get<I>()) { return std::move(v.template get<I>()); }
template<size_t I, typename T> constexpr auto getter(T const& v) -> decltype(get<I>(v)) { return get<I>(v); }
template<size_t I, typename T> constexpr auto getter(T& v) -> decltype(get<I>(v)) { return get<I>(v); }
template<size_t I, typename T> constexpr auto getter(T const&& v) -> decltype(get<I>(v)) { return std::move(get<I>(std::forward<const T>(v))); }
template<size_t I, typename T> constexpr auto getter(T&& v) -> decltype(get<I>(v)) { return std::move(get<I>(std::forward<T>(v))); }

// clang-format on

}

/// Calls `get<I>` as described in [dcl.struct.bind]\n
/// i.e. get is first search on the scope of T, if a member function cannot be
/// found to call `v.get<I>()`, ADL is performed to perform the call
/// `get<I, T>(v)`
template<size_t I, typename T> constexpr auto get(T&& arg) -> decltype(Detail::getter<I, T>(arg)) { return Detail::getter<I, T>(arg); }

template<size_t I, typename T> struct tuple_element { using type = std::remove_reference_t<decltype(get<I, T>(std::declval<T>()))>; };
template<size_t I, typename T> struct tuple_element<I, const T> { using type = std::add_const_t<typename tuple_element<I, T>::type>; };
template<size_t I, typename T> struct tuple_element<I, volatile T> { using type = std::add_volatile_t<typename tuple_element<I, T>::type>; };
template<size_t I, typename T> struct tuple_element<I, const volatile T> { using type = std::add_cv_t<typename tuple_element<I, T>::type>; };

template<size_t I, typename T> using tuple_element_t = typename tuple_element<I, T>::type;

namespace Detail {

template<typename T, size_t I, typename> struct TupleSizeHelper : public std::integral_constant<size_t, I + 1> { };

template<typename T, size_t I>
struct TupleSizeHelper<T, I, std::void_t<decltype(get<I + 1, const T>(std::declval<const T>()))>>
    : public std::integral_constant<size_t, TupleSizeHelper<T, I + 1, void>::value> { };

//template<typename T, typename U>
//struct TupleSizeHelper<std::pair<T, U>> {};

}

template<typename T> struct tuple_size : public std::integral_constant<size_t, Detail::TupleSizeHelper<T, 0, void>::value> { };
template<typename T, typename U> struct tuple_size<std::pair<T, U>> : public std::integral_constant<size_t, 2> { };
template<typename... Ts> struct tuple_size<std::tuple<Ts...>> : public std::integral_constant<size_t, sizeof...(Ts)> { };
template<typename T, size_t N> struct tuple_size<std::array<T, N>> : public std::integral_constant<size_t, N> { };

template<typename T> inline static constexpr size_t tuple_size_v = tuple_size<T>::value;

}
