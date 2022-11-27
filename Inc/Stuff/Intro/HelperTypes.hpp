#pragma once

#include <utility>
#include <variant>

namespace Stf {

struct ConvertibleToAnything {
    constexpr ConvertibleToAnything() = default;

    template<typename T> constexpr ConvertibleToAnything(const T&) { }

    template<typename T> operator T() { return std::declval<T>(); }
    template<typename T> T into() { return std::declval<T>(); }
};

template<typename... Ts> struct ABunchOfTypes {
    using tuple_type = std::tuple<Ts...>;
    using variant_type = std::variant<Ts...>;
};

template<typename... Ts> struct False {
    static const bool value = false;
};

template<size_t N> struct ArrayString {
    char str[N];

    constexpr ArrayString(char const (&str)[N]) { std::copy(str, str + N, this->str); }

    operator std::string_view() { return { str, str + N }; }
};

template<size_t N> ArrayString(char const (&)[N]) -> ArrayString<N>;

namespace Literals {
template<ArrayString G> constexpr auto operator""_arr_str() { return G; }
}

}

namespace std { // NOLINT(cert-dcl58-cpp)

template<size_t I, typename T> struct tuple_element;

template<size_t I, typename T, typename... Ts>
struct tuple_element<I, ::Stf::ABunchOfTypes<T, Ts...>> : tuple_element<I - 1, ::Stf::ABunchOfTypes<Ts...>> { };

template<typename T, typename... Ts> struct tuple_element<0, ::Stf::ABunchOfTypes<T, Ts...>> {
    using type = T;
};

}
