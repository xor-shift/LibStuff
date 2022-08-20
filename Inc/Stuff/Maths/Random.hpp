#pragma once

#include <random>

#include "./BLAS/Vector.hpp"

namespace Stf::RNG {

namespace Detail {

using erand_48_engine = std::linear_congruential_engine<uint64_t, 0x5deece66dull, 11, (1ull << 48) - 1>;

static std::random_device s_random_device {};
static thread_local std::minstd_rand s_default_engine { s_random_device() };

template<std::floating_point T> inline T range(T from, bool include_from, T to, bool include_to) {
    if (!include_from)
        from = std::nextafter(from, std::numeric_limits<T>::infinity());

    if (include_to)
        to = std::nextafter(to, std::numeric_limits<T>::infinity());

    return std::uniform_real_distribution<T> { from, to }(Detail::s_default_engine);
}

}

/// @return a T in range [0, 1]
template<typename T = float> inline T unorm() {
    const auto to = std::nextafter(static_cast<T>(1), std::numeric_limits<T>::max());
    return std::uniform_real_distribution<T> { static_cast<T>(0), to }(Detail::s_default_engine);
}

/// @return a T in range [-1, 1]
template<typename T = float> inline T snorm() {
    const auto to = std::nextafter(static_cast<T>(1), std::numeric_limits<T>::max());
    return std::uniform_real_distribution<T> { static_cast<T>(-1), to }(Detail::s_default_engine);
}

namespace Detail {

/// Box–Muller transform
template<typename T> inline Vector<T, 2> norm_impl_bm() {
    const auto y = Detail::range<T>(0, true, 1, true);
    const auto x = Detail::range<T>(0, true, 1, true);
    const auto r = std::sqrt(-2 * std::log(x));
    const auto theta = 2 * std::numbers::pi_v<T> * y;
    return { r * std::cos(theta), r * std::sin(theta) };
}

/// Marsaglia polar method (Box–Muller but good)
template<typename T> inline Vector<T, 2> norm_impl_mp() {
    const auto x = Detail::range<T>(-1, true, 1, true);
    const auto y = Detail::range<T>(-1, true, 1, true);
    const auto s = x * x + y * y;
    if (s >= 1 || s == 0)
        return norm_impl_mp<T>();
    const auto m = std::sqrt(-2 * std::log(s) / s);
    return { x * s, y * s };
}

/// C++ standard approach
template<typename T> inline Vector<T, 2> norm_impl_std() {
    thread_local static std::normal_distribution<T> dist {};
    return { dist(s_default_engine), dist(s_default_engine) };
}

}

/// @return a 2-D vector of T with both components chosen independently from the
/// normal distribution with parameters σ=1, μ=0
template<typename T = float> inline Vector<T, 2> norm() {
    const auto impl = Detail::norm_impl_std<T>;
    return std::invoke(impl);
}

// sphere samplers
namespace Detail {

template<typename T, size_t N> struct SphereSamplers {
    static inline Vector<T, N + 1> normal() {
        static constexpr size_t n_samples = ((N + 1) % 2 ? N + 2 : N + 1) / 2;
        std::array<T, n_samples * 2> samples {};
        auto it = samples.begin();
        for (auto i = 0uz; i < n_samples; i++) {
            const auto sample = norm();
            *it++ = sample[0];
            *it++ = sample[1];
        }
        Vector<T, N + 1> vec {};
        std::copy_n(samples.begin(), N + 1, vec.data);
        return Stf::vector(normalized(vec));
    }

    inline auto operator()() const { return normal(); }
};

template<typename T> struct SphereSamplers<T, 0> {
    static inline Vector<T, 1> snorm_range() {
        const auto s = Detail::s_default_engine();
        const auto min = decltype(Detail::s_default_engine)::min();
        const auto max = decltype(Detail::s_default_engine)::max();
        const auto one = static_cast<T>(1);
        if (s >= (max - min) / 2)
            return { one };
        return { -one };
    }
};

template<typename T> struct SphereSamplers<T, 1> {
    static inline Vector<T, 2> polar() {
        const auto theta = Detail::range<T>(0, true, 1, true) * std::numbers::pi_v<T> * 2;
        return { std::cos(theta), std::sin(theta) };
    }

    static inline Vector<T, 2> rejection() {
        const auto x = Detail::range<T>(-1, true, 1, true);
        const auto y = Detail::range<T>(-1, true, 1, true);
        const auto d = x * x + y * y;
        if (d >= 1 || d == 0)
            return rejection();
        return { (x * x - y * y) / d, 2 * x * y / d };
    }

    static inline Vector<T, 2> gaussian() { return normalized(norm()); }
};

template<typename T, size_t N>
inline Vector<T, N + 1> n_sphere() {
    if constexpr (N == 0) {
        return SphereSamplers<T, 0>::snorm_range();
    }

    if constexpr (N == 1) {
        return SphereSamplers<T, 1>::polar();
    }

    return SphereSamplers<T, N>::gaussian();
}

}

/// @return a sample from the surface of a unit sphere in N dimensions (the (n-1)-sphere)
template<typename T, size_t N> inline Vector<T, N> n_sphere() { return Detail::n_sphere<T, N - 1> (); }

// ball samplers
namespace Detail {

template<typename T, size_t N> struct BallSamplers {
    static inline Vector<T, N> polar_radial() {
        auto r = std::sqrt(Detail::range<T>(0, true, 1, false));
        auto sphere_sample = n_sphere<T, N - 1>();
        return Stf::vector(sphere_sample * r);
    }

    static inline Vector<T, 2> rejection()
        requires(N == 2)
    {
        auto x = Detail::range<T>(0, false, 1, false);
        auto y = Detail::range<T>(0, false, 1, false);
        if (x * x + y * y >= 1)
            return rejection();
        return { x, y };
    }

    inline auto operator()() const { return polar_radial(); }
};

template<typename T> struct BallSamplers<T, 1> {
    static inline Vector<T, 1> snorm() { return { Detail::range<T>(0, false, 1, false) }; }

    inline auto operator()() const { return snorm(); }
};

/*template<typename T> struct BallSamplers<T, 2> {
    static inline Vector<T, 2> rejection() {
        auto x = Detail::range<T>(0, false, 1, false);
        auto y = Detail::range<T>(0, false, 1, false);
        if (x * x + y * y >= 1)
            return rejection();
        return { x, y };
    }

    inline auto operator()() const { return rejection(); }
};*/

}

/// @return a sample from within a unit sphere in N dimensions (the n-ball or n-disc)
template<typename T, size_t N>
    requires(N != 0)
inline Vector<T, N> n_ball() {
    return Detail::BallSamplers<T, N> {}();
}

}
