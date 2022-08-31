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
    const auto y = range<T>(0, false, 1, false);
    const auto x = range<T>(0, false, 1, false);
    const auto r = std::sqrt(-2 * std::log(x));
    const auto theta = 2 * std::numbers::pi_v<T> * y;
    return { r * std::cos(theta), r * std::sin(theta) };
}

/// Marsaglia polar method (Box–Muller but good)
template<typename T> inline Vector<T, 2> norm_impl_mp() {
    const auto x = range<T>(-1, true, 1, true);
    const auto y = range<T>(-1, true, 1, true);
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
template<typename T> inline Vector<T, 2> norm() {
    const auto impl = Detail::norm_impl_std<T>;
    return std::invoke(impl);
}

// sphere samplers
// the notation used here is different from that in mathematics
// n signifies the dimensionality of the object itself
namespace Detail {

// !!! Partial specializations are not used to be able to benchmark general approaches in lower dimensions !!!
template<typename T, size_t N> struct SphereSamplers {
    static inline Vector<T, N> general_gaussian() {
        static constexpr size_t n_samples = (N % 2 ? N + 1 : N) / 2;
        std::array<T, n_samples * 2> samples {};
        auto it = samples.begin();
        for (auto i = 0uz; i < n_samples; i++) {
            const auto sample = norm<T>();
            *it++ = sample[0];
            *it++ = sample[1];
        }
        Vector<T, N> vec {};
        std::copy_n(samples.begin(), N, vec.data);
        return Stf::vector(normalized(vec));
    }

    static inline Vector<T, 1> snorm_range()
        requires(N == 1)
    {
        const auto u = Detail::s_default_engine();
        const auto min = decltype(Detail::s_default_engine)::min();
        const auto max = decltype(Detail::s_default_engine)::max();
        const auto one = static_cast<T>(1);
        if (u >= (max - min) / 2)
            return { one };
        return { -one };
    }

    static inline Vector<T, 2> polar()
        requires(N == 2)
    {
        const auto theta = range<T>(0, true, std::numbers::pi_v<T> * 2, false);
        return { std::cos(theta), std::sin(theta) };
    }

    static inline Vector<T, 2> rejection()
        requires(N == 2)
    {
        const auto u = range<T>(-1, true, 1, true);
        const auto v = range<T>(-1, true, 1, true);

        const auto d = u * u + v * v;

        if (d >= 1 || d == 0)
            return rejection();

        return { (u * u - v * v) / d, 2 * u * v / d };
    }

    static inline Vector<T, 2> gaussian()
        requires(N == 2)
    {
        return normalized(norm<T>());
    }

    static inline Vector<T, 3> rejection_marsaglia()
        requires(N == 3)
    {
        const auto u = range<T>(-1, true, 1, true);
        const auto v = range<T>(-1, true, 1, true);

        const auto d = u * u + v * v;

        if (d >= 1 || d == 0)
            return rejection_marsaglia();

        const auto x = 2 * u * std::sqrt(1 - d);
        const auto y = 2 * v * std::sqrt(1 - d);
        const auto z = 1 - 2 * d;

        return { x, y, z };
    }

    static inline Vector<T, 3> rejection_cook()
        requires(N == 3)
    {
        const auto x_0 = range<T>(-1, false, 1, false);
        const auto x_1 = range<T>(-1, false, 1, false);
        const auto x_2 = range<T>(-1, false, 1, false);
        const auto x_3 = range<T>(-1, false, 1, false);

        const auto x2_0 = x_0 * x_0;
        const auto x2_1 = x_1 * x_1;
        const auto x2_2 = x_2 * x_2;
        const auto x2_3 = x_3 * x_3;

        const auto d = x2_0 + x2_1 + x2_2 + x2_3;

        if (d >= 1 || d == 0)
            return rejection_cook();

        const auto x = 2 * (x_1 * x_3 + x_0 * x_2) / d;
        const auto y = 2 * (x_2 * x_3 - x_0 * x_1) / d;
        const auto z = (x2_0 + x2_3 - x2_1 - x2_2) / d;

        return { x, y, z };
    }

    static inline Vector<T, 3> polar()
        requires(N == 3)
    {
        const auto cos_theta = range<T>(-1, true, 1, false);
        const auto phi = range<T>(0, true, std::numbers::pi_v<T> * 2, false);
        const auto sin_theta = std::sin(std::acos(cos_theta));

        return {
            //
            sin_theta * std::cos(phi), //
            sin_theta * std::sin(phi), //
            cos_theta,
        };
    }

    // trades a std::sin(std::acos(val)) for a std::sqrt(1 - val * val)
    static inline Vector<T, 3> polar_noinverse()
        requires(N == 3)
    {
        const auto cos_theta = range<T>(-1, true, 1, false);
        const auto phi = range<T>(0, true, std::numbers::pi_v<T> * 2, false);
        const auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);

        return {                       //
            sin_theta * std::cos(phi), //
            sin_theta * std::sin(phi), //
            cos_theta
        };
    }
};

}

/// @return a sample from the surface of a unit sphere in N dimensions (the (n-1)-sphere)
template<typename T, size_t N>
    requires(N != 0)
inline Vector<T, N> n_sphere() {
    if constexpr (N == 1)
        return Detail::SphereSamplers<T, 1>::snorm_range();

    if constexpr (N == 2)
        return Detail::SphereSamplers<T, 2>::polar();

    if constexpr (N == 3)
        return Detail::SphereSamplers<T, 3>::polar_noinverse();

    return Detail::SphereSamplers<T, N>::general_gaussian();
}

// ball samplers
namespace Detail {

template<typename T, size_t N> struct BallSamplers {
    static inline Vector<T, N> polar_radial() {
        // WARNING TO SELF: benchmark this function again for lower dimensions if the
        // default sphere sampler is modified

        T (*root_fn)(T) = nullptr;

        if constexpr (N == 1) {
            root_fn = [](T v) { return v; };
        } else if constexpr (N == 2) {
            root_fn = [](T v) { return std::sqrt(v); };
        } else if constexpr (N == 3) {
            root_fn = [](T v) { return std::cbrt(v); };
        } else {
            root_fn = [](T v) { return std::pow(v, static_cast<T>(1) / static_cast<T>(N)); };
        }

        auto r = root_fn(range<T>(0, true, 1, false));
        auto sphere_sample = n_sphere<T, N>();
        return Stf::vector(sphere_sample * r);
    }

    static inline Vector<T, N> general_rejection() {
        Vector<T, N> vec;
        for (auto i = 0uz; i < N; i++)
            vec.data[i] = range<T>(-1, true, 1, false);
        if (magnitude_squared(vec) >= 1)
            return general_rejection();
        return vec;
    }

    static inline Vector<T, 1> snorm()
        requires(N == 1)
    {
        return { range<T>(-1, true, 1, false) };
    }

    static inline Vector<T, 2> rejection()
        requires(N == 2)
    {
        const auto x = range<T>(-1, true, 1, false);
        const auto y = range<T>(-1, true, 1, false);
        if (x * x + y * y >= 1)
            return rejection();
        return { x, y };
    }

    static inline Vector<T, 2> concentric()
        requires(N == 2)
    {
        const auto u = range<T>(-1, true, 1, false);
        const auto v = range<T>(-1, true, 1, false);
        if (u == 0 && v == 0)
            return { 0, 0 };

        const auto theta = (u * u > v * v)          //
            ? (std::numbers::pi_v<T> / 4) * (v / u) //
            : (std::numbers::pi_v<T> / 2) - (std::numbers::pi_v<T> / 4) * (u / v);
        const auto r = (u * u > v * v) ? u : v;

        return { r * std::cos(theta), r * std::sin(theta) };
    }
};

}

/// @return a sample from within a unit sphere in N dimensions (the n-ball or n-disc)
template<typename T, size_t N>
    requires(N != 0)
inline Vector<T, N> n_ball() {
    if constexpr (N == 1)
        return Detail::BallSamplers<T, 1>::snorm();

    if constexpr (N == 2)
        return Detail::BallSamplers<T, 2>::concentric();

    return Detail::BallSamplers<T, N>::polar_radial();
}

}

namespace Stf::RNGNew {

struct RNGContext {};

}
