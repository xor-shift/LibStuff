#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace Stf {

constexpr size_t hash_combine(size_t lhs, size_t rhs) {
    [[maybe_unused]] constexpr size_t k_magic_phi = sizeof(size_t) == sizeof(uint64_t) ? 0x9e3779b97f4a7c15ull : 0x9e3779b9ul;
    [[maybe_unused]] constexpr size_t k_magic_prime = sizeof(size_t) == sizeof(uint64_t) ? 0x9e3779b97f4a7c55ull : 0x9e3779b1ul;

    return lhs ^ (rhs + k_magic_prime + (lhs << 6) + (lhs >> 2));
}

template<typename T, typename... Ts> constexpr size_t multi_hash(const T& v, const Ts&... vs) {
    std::size_t lhs = std::hash<T> {}(v);

    if constexpr (sizeof...(Ts) > 0)
        return hash_combine(lhs, multi_hash(vs...));
    else
        return lhs;
}

template<typename It>
constexpr size_t iter_hash(It beg, It end) {
    using HashFn = std::hash<typename It::value_type>;
    HashFn hash_fn {};

    size_t hash = 0;

    for (It it = beg; it != end;) {
        hash = hash_combine(hash, hash_fn(*it++));
    }

    return hash;
}

}

namespace std {

template<typename T, typename U> struct hash<pair<T, U>> { // NOLINT(cert-dcl58-cpp)
    using argument_type = pair<T, U>;
    using result_type = size_t;

    constexpr result_type operator()(argument_type const& v) const {
        const auto lhs = hash<T> {}(v.first);
        const auto rhs = hash<U> {}(v.second);
        return ::Stf::hash_combine(lhs, rhs);
    }
};

template<typename... Ts> struct hash<tuple<Ts...>> { // NOLINT(cert-dcl58-cpp)
    using argument_type = tuple<Ts...>;
    using result_type = size_t;

    constexpr result_type operator()(argument_type const& v) const {
        auto seq = make_index_sequence<sizeof...(Ts)> {};
        return impl(v, seq);
    }

private:
    template<size_t... Is>
    constexpr result_type impl(argument_type const& v, integer_sequence<size_t, Is...>) const {
        return ::Stf::multi_hash(std::get<Is>(v)...);
    }
};

template<typename T, size_t N> struct hash<array<T, N>> { // NOLINT(cert-dcl58-cpp)
    using argument_type = array<T, N>;
    using result_type = size_t;

    constexpr result_type operator()(argument_type const& v) const {
        if constexpr (N == 0)
            return 0;

        const hash<T> hasher{};

        result_type ret = hasher(v[0]);

        if constexpr (N == 1)
            return ret;

        for (size_t i = 1; i < N; i++)
            ret = ::Stf::hash_combine(ret, hasher(v[i]));

        return ret;
    }
};

}
