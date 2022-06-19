#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Stf::Refl {

template<typename> struct is_refl : public std::false_type { };

// all std::__is_tuple_like<T>s are also refls

template<typename... Ts> struct is_refl<std::tuple<Ts...>> : public std::true_type { };

template<typename T, typename U> struct is_refl<std::pair<T, U>> : public std::true_type { };

// template<typename T, size_t n>
// struct is_refl<std::array<T, n>> : public std::true_type {};

};

#define MEMREFL_BEGIN(name, count)                                                                                  \
    template<size_t R_I> constexpr auto const& get() const& { return MemReflHelper<R_I>::get(*this); }              \
    template<size_t R_I> constexpr auto& get()& { return MemReflHelper<R_I>::get(*this); }                          \
    template<size_t R_I> constexpr auto const&& get() const&& { return std::move(MemReflHelper<R_I>::get(*this)); } \
    template<size_t R_I> constexpr auto&& get()&& { return std::move(MemReflHelper<R_I>::get(*this)); }             \
                                                                                                                    \
    template<size_t R_I, typename = void> struct MemReflHelper;                                                     \
    struct MemReflBaseHelper {                                                                                      \
        using parent_type = name;                                                                                   \
                                                                                                                    \
        static constexpr size_t member_count = count;                                                               \
        static constexpr size_t ct_base = __COUNTER__;                                                              \
    };

/*

//GCC workaround for " error: explicit specialization in non-namespace scope":
struct Foo {
    template<size_t, typename = void> struct Bar;
    template<size_t I> struct Bar<I, std::enable_if_t<I == 0>>;
};

//Below doesn't work:
struct Foo {
    template<size_t> struct Bar;
    template<> struct Bar<0>;
};

*/

#define MEMREFL_MEMBER(name)                                                                                                                            \
    template<size_t R_I> struct MemReflHelper<R_I, std::enable_if_t<R_I == (__COUNTER__ - MemReflBaseHelper::ct_base - 1)>> {                           \
        using value_type = decltype(MemReflBaseHelper::parent_type::name);                                                                              \
        static constexpr std::string_view member_name = #name;                                                                                          \
                                                                                                                                                        \
        static constexpr value_type const& get(typename MemReflBaseHelper::parent_type const& p) { return p.name; }                                     \
        static constexpr value_type const&& get(typename MemReflBaseHelper::parent_type const&& p) { return std::move(p.name); }                        \
                                                                                                                                                        \
        static constexpr std::enable_if_t<!std::is_const_v<value_type>, value_type&> get(typename MemReflBaseHelper::parent_type& p) { return p.name; } \
                                                                                                                                                        \
        static constexpr std::enable_if_t<!std::is_const_v<value_type>, value_type&&> get(typename MemReflBaseHelper::parent_type&& p) {                \
            return std::move(p.name);                                                                                                                   \
        }                                                                                                                                               \
    };

#define MEMREFL_DECL_MEMBER(name) \
    name {};                      \
    MEMREFL_MEMBER(name)

namespace Stf::Refl {

template<typename T> struct ExtReflBaseHelper;

template<size_t I, typename T> struct ExtReflHelper;

}

#define EXTREFL_BEGIN(name, count)                     \
    namespace Stf::Refl {                              \
    template<> struct ExtReflBaseHelper<name> {        \
        static constexpr size_t member_count = count;  \
        static constexpr size_t ct_base = __COUNTER__; \
    };                                                 \
    }

#define EXTREFL_MEMBER(name, member)                                                                                                \
    namespace Stf::Refl {                                                                                                           \
    template<> struct ExtReflHelper<__COUNTER__ - ExtReflBaseHelper<name>::ct_base - 1, name> {                                     \
        using value_type = decltype(name::member);                                                                                  \
        static constexpr std::string_view member_name = #name;                                                                      \
                                                                                                                                    \
        static constexpr value_type const& get(name const& p) { return p.member; }                                                  \
                                                                                                                                    \
        static constexpr std::enable_if_t<!std::is_const_v<value_type>, value_type&> get(name& p) { return p.member; }              \
                                                                                                                                    \
        static constexpr std::enable_if_t<!std::is_const_v<value_type>, value_type&&> get(name&& p) { return std::move(p.member); } \
    };                                                                                                                              \
    }

namespace Stf::Refl {

template<typename T>
concept InternallyReflectable = requires(T const& self, T& mut_self, T&& rvref_self) {
                                    // typename std::enable_if_t<std::is_same_v<typename T::serialization_tag, MemberSerializationTag>, void>;
                                    typename std::enable_if_t<std::is_same_v<T, typename T::MemReflBaseHelper::parent_type>, void>;
                                    { T::MemReflBaseHelper::member_count } -> std::convertible_to<size_t>;
                                };

template<typename T>
concept ExternallyReflectable = requires(T const& self, T& mut_self, T&& rvref_self) {
                                    typename ExtReflBaseHelper<T>;
                                    { ExtReflBaseHelper<T>::member_count } -> std::convertible_to<size_t>;
                                };

template<typename T>
concept TupleLike = requires(T const& self, T& mut_self, T&& rvref_self) {
                        typename std::enable_if_t<!InternallyReflectable<T>>;
                        typename std::enable_if_t<!ExternallyReflectable<T>>;
                        { std::tuple_size<T>::value } -> std::convertible_to<size_t>;
                        /*typename std::tuple_element_t<std::tuple_size<T>::value - 1, T>;
                        typename std::enable_if_t<
                            !std::is_same_v<
                                std::array<
                                    std::tuple_element_t<std::tuple_size<T>::value - 1, T>, std::tuple_size<T>::value>,
                                T>>;*/
                    };

template<typename T>
concept Reflectable = (InternallyReflectable<T> || ExternallyReflectable<T> /* || TupleLike<T>*/);

template<Reflectable T> struct is_refl<T> : std::true_type { };

}

namespace Stf::Refl {

template<typename T> struct member_count;

template<size_t I, typename T> struct member_type;

template<size_t I, typename T> struct member_name { static constexpr const char* value = "<anonymous>"; };

/// Internally reflectable types ///

template<InternallyReflectable T> struct member_count<T> : std::integral_constant<size_t, T::MemReflBaseHelper::member_count> { };

template<size_t I, InternallyReflectable T> struct member_name<I, T> { static constexpr auto value = T::template MemReflHelper<I>::member_name; };

template<size_t I, InternallyReflectable T> struct member_type<I, T> { using type = typename T::template MemReflHelper<I>::value_type; };

template<size_t I, InternallyReflectable T> constexpr auto const& get(T const& t) { return T::template MemReflHelper<I>::get(t); }

template<size_t I, InternallyReflectable T> constexpr auto& get(T& t) { return T::template MemReflHelper<I>::get(t); }

template<size_t I, InternallyReflectable T> constexpr auto&& get(T&& t) { return std::move(T::template MemReflHelper<I>::get(t)); }

/// Externally reflectable types ///

template<ExternallyReflectable T> struct member_count<T> : std::integral_constant<size_t, ExtReflBaseHelper<T>::member_count> { };

template<size_t I, ExternallyReflectable T> struct member_name<I, T> { static constexpr auto value = ExtReflHelper<I, T>::member_name; };

template<size_t I, ExternallyReflectable T> struct member_type<I, T> { using type = typename ExtReflHelper<I, T>::value_type; };

template<size_t I, ExternallyReflectable T> constexpr auto const& get(T const& t) { return ExtReflHelper<I, T>::get(t); }

template<size_t I, ExternallyReflectable T> constexpr auto& get(T& t) { return ExtReflHelper<I, T>::get(t); }

template<size_t I, ExternallyReflectable T> constexpr auto&& get(T&& t) { return std::move(ExtReflHelper<I, T>::get(t)); }

/// Coincidentally reflectable types ///

/*template<TupleLike T>
struct member_count<T> : std::tuple_size<T> { };

template<size_t I, TupleLike T> struct member_name<I, T> {
    static constexpr auto value = "<anonymous field>";
};

template<size_t I, TupleLike T> struct member_type<I, T> : public std::tuple_element<I, T> {};

template<size_t I, TupleLike T> constexpr auto const& get(T const& t) {
    return std::get<I>(t);
}

template<size_t I, TupleLike T> constexpr auto& get(T& t) { return std::get<I>(t); }

template<size_t I, TupleLike T> constexpr auto&& get(T&& t) {
    return std::move(std::get<I>(t));
}*/

/// Fluff ///

template<size_t I, Reflectable T> inline static constexpr auto member_name_v = member_name<I, T>::value;

template<Reflectable T> inline static constexpr auto member_count_v = member_count<T>::value;

template<size_t I, Reflectable T> using member_type_t = typename member_type<I, T>::type;

template<size_t I, Reflectable T> constexpr std::string_view get_member_name(T const&) { return member_name_v<I, T>; }

}

namespace std {

template<::Stf::Refl::Reflectable T> struct tuple_size<T> : integral_constant<size_t, ::Stf::Refl::member_count<T>::value> { };

template<size_t I, ::Stf::Refl::Reflectable T> struct tuple_element<I, T> : public ::Stf::Refl::member_type<I, T> { };

template<size_t I, ::Stf::Refl::Reflectable T> constexpr auto const& get(T const& t) { return ::Stf::Refl::get<I>(t); }

template<size_t I, ::Stf::Refl::InternallyReflectable T> constexpr auto& get(T& t) { return ::Stf::Refl::get<I>(t); }

template<size_t I, ::Stf::Refl::InternallyReflectable T> constexpr auto&& get(T&& t) { return ::Stf::Refl::get<I>(std::forward<T>(t)); }

}
