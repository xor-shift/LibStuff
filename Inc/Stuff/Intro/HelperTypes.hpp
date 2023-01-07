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

    static constexpr size_t size = sizeof...(Ts);
};

template<auto... Vs> struct ABunchOfValues;

template<> struct ABunchOfValues<> {
    static constexpr size_t size = 0;
};

template<auto V, auto... Vs> struct ABunchOfValues<V, Vs...> {
    static constexpr size_t size = sizeof...(Vs) + 1;

    static constexpr auto head = V;
    using tail_type = std::conditional_t<sizeof...(Vs) == 0, void, ABunchOfValues<Vs...>>;

    template<size_t N> static constexpr auto get() {
        if constexpr (N == 0)
            return V;
        else
            return tail_type::template get<N - 1>();
    }
};

template<typename... Ts> struct False {
    static const bool value = false;
};

template<size_t N> struct ArrayString {
    char str[N];

    constexpr ArrayString(char const (&str)[N]) { std::copy(str, str + N, this->str); }

    operator std::string_view() const { return { str, str + N - 1 }; }

    template<size_t M>
    friend constexpr std::strong_ordering operator<=>(ArrayString const& lhs, ArrayString<M> const& rhs) {
        for (size_t i = 0; i < std::min(M, N); i++) {
            std::strong_ordering t = lhs.str[i] <=> rhs.str[i];
            if (t != std::strong_ordering::equal)
                return t;
        }

        return N <=> M;
    }
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
