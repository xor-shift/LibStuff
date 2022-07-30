#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include <Stuff/Serde/Concepts.hpp>
#include <Stuff/Util/Visitor.hpp>

namespace Stf::Serde::New {

template<std::integral T, Serializer S> constexpr void serialize(T v, S& serializer) { serializer.serialize_primitive(v); }
template<std::floating_point T, Serializer S> constexpr void serialize(T v, S& serializer) { serializer.serialize_primitive(v); }

template<Serializer S> constexpr void serialize(__int128_t v, S& serializer) { serializer.serialize_i128(v); }
template<Serializer S> constexpr void serialize(__uint128_t v, S& serializer) { serializer.serialize_u128(v); }
template<Serializer S> constexpr void serialize(bool v, S& serializer) { serializer.serialize_bool(v); }
template<Serializer S> constexpr void serialize(char v, S& serializer) { serializer.serialize_char(v); }
template<Serializer S> constexpr void serialize(wchar_t v, S& serializer) { serializer.serialize_char(v); }

template<typename CharType, Serializer S, typename Traits = std::char_traits<CharType>>
constexpr void serialize(std::basic_string_view<CharType, Traits> str, S& serializer) {
    serializer.serialize_string(str);
}

template<typename CharType, Serializer S, typename Traits = std::char_traits<CharType>, typename Alloc = std::allocator<CharType>>
constexpr void serialize(std::basic_string<CharType, Traits, Alloc> str, S& serializer) {
    serialize(std::string_view(str), serializer);
}

template<Serializer S> constexpr void serialize(const char* str, S& serializer) { serialize(std::string_view(str), serializer); }
template<Serializer S> constexpr void serialize(const wchar_t* str, S& serializer) { serialize(std::basic_string_view(str), serializer); }
template<Serializer S> constexpr void serialize(const char8_t* str, S& serializer) { serialize(std::basic_string_view(str), serializer); }
template<Serializer S> constexpr void serialize(const char16_t* str, S& serializer) { serialize(std::basic_string_view(str), serializer); }
template<Serializer S> constexpr void serialize(const char32_t* str, S& serializer) { serialize(std::basic_string_view(str), serializer); }

template<std::ranges::sized_range T, Serializer S> constexpr void serialize(T const& range, S& serializer) {
    typename S::ser_sequence_type sequence_serializer = serializer.serialize_sequence(std::ranges::size(range));

    const auto begin = std::ranges::begin(range);
    const auto end = std::ranges::end(range);
    for (auto it = begin; it != end; it++) {
        using U = std::decay_t<decltype(*it)>;

        sequence_serializer.template serialize_element<U>(*it);
    }

    sequence_serializer.end_sequence();
}

namespace Detail {

template<typename TupleLike, Serializer S, size_t I = 0> constexpr void serialize_tuple_like(TupleLike const& t, S& serializer) {
    constexpr auto t_size = std::tuple_size_v<TupleLike>;

    if constexpr (t_size == 0) {
        return;
    } else {
        auto const& v = std::get<I>(t);
        using U = std::tuple_element_t<I, TupleLike>;
        serializer.template serialize_element<U>(v);

        if constexpr (I + 1 < t_size)
            return serialize_tuple_like<TupleLike, S, I + 1>(t, serializer);
    }
}

}

template<typename T, typename U, Serializer S> constexpr void serialize(std::pair<T, U> const& v, S& serializer) {
    typename S::ser_sequence_type tuple_adapter = serializer.serialize_tuple(2);
    Detail::serialize_tuple_like(v, tuple_adapter);
    tuple_adapter.end_sequence();
}

template<typename... Ts, Serializer S> constexpr void serialize(std::tuple<Ts...> const& v, S& serializer) {
    typename S::ser_sequence_type tuple_adapter = serializer.serialize_tuple(sizeof...(Ts));
    Detail::serialize_tuple_like(v, tuple_adapter);
    tuple_adapter.end_sequence();
}

namespace Detail {

template<typename T, Serializer S, size_t I = 0> constexpr void serialize_reflectable_impl(T const& v, S& serializer) {
    const auto field_info = Serde::New::field_information<I>(v);
    auto const& field = v.*(field_info.member_pointer);
    const auto field_name = field_info.name;

    serializer.serialize_key(field_name);
    serializer.serialize_value(field);

    if constexpr (I + 1 < Stf::Serde::New::tuple_size_v<T>)
        return serialize_reflectable_impl<T, S, I + 1>(v, serializer);
}

}

template<typename T, Serializer S>
    requires(requires { typename std::void_t<decltype(Serde::New::get<0, T>(std::declval<T>()))>; })
constexpr void serialize(T const& v, S& serializer) {
    typename S::ser_map_type struct_serializer = serializer.serialize_struct(tuple_size_v<T>, struct_name(v));
    Detail::serialize_reflectable_impl(v, struct_serializer);
    struct_serializer.end_map();
}

template<typename T, Serializer S> constexpr void serialize(std::optional<T> const& opt, S& serializer) {
    if (!(bool)opt) {
        serializer.serialize_nullopt();
        return;
    }

    T const& v = *opt;
    serializer.serialize_some(v);
}

template<typename... Ts, Serializer S> constexpr void serialize(std::variant<Ts...> const& v, S& serializer) {
    if (v.valueless_by_exception()) {
        serializer.serialize_monostate();
    } else {
        const auto visitor = MultiVisitor {
            [&serializer](std::monostate) {
                // this comment is for clang-format to behave
                serializer.serialize_monostate();
            },
            [&serializer, &va = v]<typename T>(T const& v) {
                const auto index = va.index();
                if (index == std::variant_npos)
                    std::unreachable();
                serializer.template serialize_variant<T>(v, index);
            }};

        std::visit(visitor, v);
    }
}

}
