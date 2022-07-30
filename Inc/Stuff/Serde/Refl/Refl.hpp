#pragma once

#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace Stf::Serde::New {

template<typename Base, typename T> struct FieldInformation {
    T Base::*member_pointer;
    std::string_view name;
    std::string_view attributes;

    constexpr bool is_anonymous() const { return member_pointer == nullptr && name == "<anonymous>" && attributes == ""; }
};

namespace Detail {

struct AnonymousFieldSentinel { };

template<typename T> constexpr FieldInformation<Detail::AnonymousFieldSentinel, T> make_anonymous_field() {
    return {
        .member_pointer = nullptr,
        .name = "<anonymous>",
        .attributes = "",
    };
}

}

template<typename T> inline static constexpr auto anonymous_field_v = Detail::make_anonymous_field<T>();

namespace Detail {

template<typename T> constexpr auto get_refl_helper(T const& v) -> decltype(_get_refl_helper(v)) { return _get_refl_helper(v); }
template<typename T> constexpr auto get_refl_helper(T const& v) -> decltype(T::_refl_helper) { return v._refl_helper; }

template<typename Base, typename... MemberTypes> struct ReflHelper {
    std::string_view struct_name;
    std::tuple<MemberTypes Base::*...> member_pointers;
    std::array<std::string_view, sizeof...(MemberTypes)> names;
    std::array<std::string_view, sizeof...(MemberTypes)> attributes;

    template<size_t N>
        requires(sizeof...(MemberTypes) != 0)
    using member_type = std::tuple_element_t<N, std::tuple<MemberTypes...>>;

    template<typename U> constexpr ReflHelper<Base, MemberTypes..., U> operator()(U Base::*ptr, std::string_view name = "", std::string_view attribs = "") {
        auto tuple = std::tuple_cat(std::move(member_pointers), std::tuple<U Base::*> { ptr });

        std::array<std::string_view, sizeof...(MemberTypes) + 1> new_attributes;
        std::copy(attributes.begin(), attributes.end(), new_attributes.begin());
        new_attributes.back() = attribs;
        std::array<std::string_view, sizeof...(MemberTypes) + 1> new_names;
        std::copy(names.begin(), names.end(), new_names.begin());
        new_names.back() = name;

        return {
            .struct_name = struct_name,
            .member_pointers = std::move(tuple),
            .names = new_names,
            .attributes = new_attributes,
        };
    }

    template<size_t N>
        requires(N < sizeof...(MemberTypes))
    constexpr auto get() const {
        return std::get<N>(member_pointers);
    }
    template<size_t N>
        requires(N < sizeof...(MemberTypes))
    constexpr auto attribs() const {
        return std::get<N>(attributes);
    }

    template<size_t N>
        requires(N < sizeof...(MemberTypes))
    constexpr FieldInformation<Base, member_type<N>> information() const {
        return {
            .member_pointer = std::get<N>(member_pointers),
            .name = std::get<N>(names),
            .attributes = std::get<N>(attributes),
        };
    }
};

}

template<size_t N, typename T> auto get(T const& v) -> decltype(v.*Detail::get_refl_helper<T>(v).template get<N>()) const& {
    return v.*Detail::get_refl_helper<T>(v).template get<N>();
}
template<size_t N, typename T> auto get(T const&& v) -> decltype(std::move(v.*Detail::get_refl_helper<T>(v).template get<N>())) const&& {
    return std::move(v.*Detail::get_refl_helper<T>(v).template get<N>());
}
template<size_t N, typename T> auto get(T& v) -> decltype(v.*Detail::get_refl_helper<T>(v).template get<N>())& {
    return v.*Detail::get_refl_helper<T>(v).template get<N>();
}
template<size_t N, typename T> auto get(T&& v) -> decltype(std::move(v.*Detail::get_refl_helper<T>(v).template get<N>()))&& {
    return std::move(v.*Detail::get_refl_helper<T>(v).template get<N>());
}

template<size_t N, typename T> auto field_information(T const& v) { return Detail::get_refl_helper<T>(v).template information<N>(); }
template<typename T> std::string_view struct_name(T const& v) { return Detail::get_refl_helper<T>(v).struct_name; }

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

// template<typename T, typename U>
// struct TupleSizeHelper<std::pair<T, U>> {};

}

template<typename T> struct tuple_size : public std::integral_constant<size_t, Detail::TupleSizeHelper<T, 0, void>::value> { };
// template<typename T, typename U> struct tuple_size<std::pair<T, U>> : public std::integral_constant<size_t, 2> { };
// template<typename... Ts> struct tuple_size<std::tuple<Ts...>> : public std::integral_constant<size_t, sizeof...(Ts)> { };
// template<typename T, size_t N> struct tuple_size<std::array<T, N>> : public std::integral_constant<size_t, N> { };

template<typename T> inline static constexpr size_t tuple_size_v = tuple_size<T>::value;

}

#define INT_REFL(_name, ...)                                                                 \
    inline static constexpr auto _refl_helper = Stf::Serde::New::Detail::ReflHelper<_name> { \
        .struct_name = #_name,                                                               \
    } __VA_ARGS__

#define EXT_REFL(_name, ...)                                                                    \
    template<typename T> struct _ReflHelperGetter;                                              \
                                                                                                \
    template<> struct _ReflHelperGetter<_name> {                                                \
        inline static constexpr auto refl_helper = Stf::Serde::New::Detail::ReflHelper<_name> { \
            .struct_name = #_name,                                                              \
        } __VA_ARGS__;                                                                          \
        using helper_type = decltype(refl_helper);                                              \
    };                                                                                          \
                                                                                                \
    static constexpr _ReflHelperGetter<_name>::helper_type const& _get_refl_helper(_name const&) { return _ReflHelperGetter<_name>::refl_helper; }
