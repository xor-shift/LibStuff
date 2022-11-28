#pragma once

#include <Stuff/Util/Hacks/Concepts.hpp>
#include <Stuff/Util/Hacks/Expected.hpp>
#include <Stuff/Util/Hacks/Try.hpp>

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>

#include <Stuff/Util/Hash.hpp>
#include <Stuff/Util/Tuple.hpp>

#include <Stuff/Intro/Introspectors/Array.hpp>
#include <Stuff/Intro/Introspectors/Map.hpp>
#include <Stuff/Intro/Introspectors/Span.hpp>
#include <Stuff/Intro/Introspectors/Tuple.hpp>
#include <Stuff/Intro/Introspectors/UnorderedMap.hpp>
#include <Stuff/Intro/Introspectors/Vector.hpp>

#include <Stuff/Intro/Dump.hpp>

#include <Stuff/Intro/Introspector.hpp>

namespace Stf::Comp {

namespace Detail {

// N-dimensional Unordered Map Array Helper
template<typename T, size_t N> struct NUMArrHelper;

template<typename T> struct NUMArrHelper<T, 1> {
    using type = std::unordered_map<ptrdiff_t, T>;
};

template<typename T, size_t N> struct NUMArrHelper {
    using type = std::unordered_map<ptrdiff_t, typename NUMArrHelper<T, N - 1>::type>;
};

}

namespace Aliases {

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using i128 = __int128_t;
using isize = ptrdiff_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u128 = __uint128_t;
using usize = size_t;

template<typename T> using span = std::span<T>;

template<typename... Ts> using umap = std::unordered_map<Ts...>;

template<typename T> using umarr = umap<isize, T>;

template<typename T, size_t N> using numarr = typename Detail::NUMArrHelper<T, N>::type;

template<typename... Ts> using vec = std::vector<Ts...>;

template<typename T, size_t N> using arr = std::array<T, N>;

template<typename T> using opt = std::optional<T>;

}

template<typename T, typename Pred> constexpr std::optional<T> opt_if(T v, Pred&& pred) {
    if (std::invoke(std::forward<Pred>(pred), std::forward<T>(v)))
        return std::move(v);
    return std::nullopt;
}

namespace Detail {

template<typename T, typename Fn>
    requires(!requires(Fn && fn, std::span<T> v) { std::ignore = std::invoke(std::forward<Fn>(fn), v); })
constexpr void windowed_foreach(size_t window_size, std::span<T> vals, Fn&& fn) {
    if (window_size == 0)
        return;

    for (size_t i = 0; i < vals.size() + window_size; i++) {
        const auto beg = std::clamp(i, window_size, vals.size() + window_size) - window_size;
        const auto end = std::clamp(i + window_size, window_size, vals.size() + window_size) - window_size;
        if (end == beg)
            continue;
        std::invoke(std::forward<Fn>(fn), std::span(vals.data() + beg, end - beg));
    }
}

template<typename T, typename Fn>
    requires(requires(Fn && fn, std::span<T> v) { std::ignore = std::invoke(std::forward<Fn>(fn), v); })
constexpr auto windowed_foreach(size_t window_size, std::span<T> vals, Fn&& fn) {
    using Ret = std::invoke_result_t<Fn, std::span<T>>;
    std::vector<Ret> vec {};
    windowed_foreach(window_size, vals, [&](auto v) {
        vec.template emplace_back(std::forward<Ret>(std::invoke(std::forward<Fn>(fn), v)));
    });
    return vec;
}

}

template<typename T, typename Fn> constexpr auto windowed_foreach(size_t window_size, T&& container, Fn&& fn) {
    return Detail::windowed_foreach(window_size, std::span(container.data(), container.size()), std::forward<Fn>(fn));
}

}