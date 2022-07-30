#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <optional>
#include <span>
#include <tuple>
#include <utility>

#include <Stuff/Util/DummyIterator.hpp>

#include "./ReflNew.hpp"

namespace Stf {

namespace Detail {

template<typename T, typename It, typename = void> struct Serializer;

template<typename T, typename It> struct Serializer<T, It, std::enable_if_t<std::is_arithmetic_v<T>>> {
    static constexpr It serialize(It it, T const& v) {
        // const auto res = __builtin_bit_cast(std::array<char, sizeof(T)>, v);
        const auto res = std::bit_cast<std::array<uint8_t, sizeof(T)>>(v);
        if constexpr (std::endian::native == std::endian::big)
            return std::copy(res.cbegin(), res.cend(), it);
        else
            return std::reverse_copy(res.cbegin(), res.cend(), it);
    }

    static constexpr It deserialize(T& v, It it) {
        std::array<uint8_t, sizeof(T)> arr {};
        if constexpr (std::endian::native == std::endian::big)
            std::copy(it, it + sizeof(T), arr.begin());
        else
            std::reverse_copy(it, it + sizeof(T), arr.begin());
        v = std::bit_cast<T>(arr);
        return it + sizeof(T);
    }
};

template<typename T, typename It> struct Serializer<T, It, std::enable_if_t<std::is_enum_v<T>>> {
    using U = std::underlying_type_t<T>;

    static constexpr It serialize(It it, T const& v) {
        ;
        return Serializer<U, It>::serialize(it, static_cast<U>(v));
    }

    static constexpr It deserialize(T& v, It it) {
        U v_2 {};
        it = Serializer<U, It>::deserialize(v_2, it);
        v = static_cast<T>(v_2);
        return it;
    }
};

template<typename T, typename It, size_t n> struct Serializer<T[n], It> {
    static constexpr It serialize(It it, T const (&v)[n]) {
        for (size_t i = 0; i < n; i++)
            it = Serializer<T, It>::serialize(it, v[i]);

        return it;
    }

    static constexpr It deserialize(T (&v)[n], It it) {
        for (size_t i = 0; i < n; i++)
            it = Serializer<T, It>::deserialize(v[i], it);

        return it;
    }
};

template<typename T, typename It, size_t n> struct Serializer<std::array<T, n>, It> {
    static constexpr It serialize(It it, std::array<T, n> const& v) {
        for (size_t i = 0; i < n; i++)
            it = Serializer<T, It>::serialize(it, v[i]);
        return it;
    }

    static constexpr It deserialize(std::array<T, n>& v, It it) {
        for (size_t i = 0; i < n; i++)
            it = Serializer<T, It>::deserialize(v[i], it);

        return it;
    }
};

template<typename T, typename It> struct Serializer<std::optional<T>, It> {
    static constexpr It serialize(It it, std::optional<T> const& v) {
        it = Serializer<bool, It>::serialize(it, v.has_value());
        return Serializer<T, It>::serialize(it, v ? *v : T {});
    }

    static constexpr It deserialize(std::optional<T>& v, It it) {
        bool armed = false;
        T v_2 {};

        it = Serializer<bool, It>::deserialize(armed, it);
        it = Serializer<T, It>::deserialize(v_2, it);

        if (armed)
            v = v_2;

        return it;
    }
};

template<typename T, typename It> struct Serializer<T, It, std::void_t<decltype(Refl::get<0, T>)>> {
    static constexpr It serialize(It it, T const& v) { return serialize_impl<0>(it, v); }

    static constexpr It deserialize(T& v, It it) { return deserialize_impl<0>(v, it); }

private:
    template<size_t i = 0> static constexpr It serialize_impl(It it, T const& v) {
        using U = Stf::Refl::tuple_element_t<i, T>;

        it = Serializer<U, It>::serialize(it, Stf::Refl::get<i>(v));

        if constexpr (i + 1 < Stf::Refl::tuple_size_v<T>)
            return serialize_impl<i + 1>(it, v);
        else
            return it;
    }

    template<size_t i = 0> static constexpr It deserialize_impl(T& v, It it) {
        using U = Stf::Refl::tuple_element_t<i, T>;

        it = Serializer<U, It>::deserialize(Stf::Refl::get<i>(v), it);

        if constexpr (i + 1 < Stf::Refl::tuple_size_v<T>)
            return deserialize_impl<i + 1>(v, it);
        else
            return it;
    }
};

template<typename T, typename It> struct Serializer<T const&, It> { };

}

template<typename T, typename It> constexpr It serialize(It it, T const& v) { return Detail::Serializer<T, It>::serialize(it, v); }

template<typename T, typename It> constexpr It deserialize(T& v, It it) { return Detail::Serializer<T, It>::deserialize(v, it); }

namespace Detail {

template<typename T> constexpr std::enable_if_t<std::is_default_constructible_v<T>, ptrdiff_t> serialized_size() {
    DummyIterator<uint8_t> start {};
    return serialize(start, T {}) - start;
}

}

template<typename T> inline constexpr size_t serialized_size_v = Detail::serialized_size<T>();

template<typename T> std::array<uint8_t, serialized_size_v<T>> serialize(T const& v) {
    std::array<uint8_t, serialized_size_v<T>> arr {};
    serialize(arr.begin(), v);
    return arr;
}

template<typename T, typename It> constexpr T deserialize(It it) {
    T v {};
    std::ignore = Detail::Serializer<T, It>::deserialize(v, it);
    return v;
}

}
