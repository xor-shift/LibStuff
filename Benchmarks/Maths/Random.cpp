#include <benchmark/benchmark.h>

#include <Stuff/Maths/Random.hpp>

template<typename Sampler> static void common(benchmark::State& state, Sampler&& sampler) {
    for (auto _ : state) {
        const auto sample = std::invoke(sampler);
        benchmark::DoNotOptimize(sample);
    }
}

#define MAKE_BENCH(_name, _sampler)                                                \
    static void _name(benchmark::State& state) { return common(state, _sampler); } \
    BENCHMARK(_name)

#define MAKE_GENERAL_BENCH(_category, _name, _type, _struct, _sampler) \
    MAKE_BENCH(_category##_1##_gnrl_##_name, (_struct<_type, 1>::_sampler));     \
    MAKE_BENCH(_category##_2##_gnrl_##_name, (_struct<_type, 2>::_sampler));     \
    MAKE_BENCH(_category##_3##_gnrl_##_name, (_struct<_type, 3>::_sampler));     \
    MAKE_BENCH(_category##_4##_gnrl_##_name, (_struct<_type, 4>::_sampler))

MAKE_BENCH(normal_bm, Stf::RNG::Detail::norm_impl_bm<float>);
MAKE_BENCH(normal_mp, Stf::RNG::Detail::norm_impl_mp<float>);
MAKE_BENCH(normal_std, Stf::RNG::Detail::norm_impl_std<float>);

MAKE_BENCH(sphere_1_spec_snorm, (Stf::RNG::Detail::SphereSamplers<float, 1>::snorm_range));
MAKE_BENCH(sphere_2_spec_polar, (Stf::RNG::Detail::SphereSamplers<float, 2>::polar));
MAKE_BENCH(sphere_2_spec_rejection, (Stf::RNG::Detail::SphereSamplers<float, 2>::rejection));
MAKE_BENCH(sphere_2_spec_gaussian, (Stf::RNG::Detail::SphereSamplers<float, 2>::gaussian));
MAKE_BENCH(sphere_3_spec_rejection_marsaglia, (Stf::RNG::Detail::SphereSamplers<float, 3>::rejection_marsaglia));
MAKE_BENCH(sphere_3_spec_rejection_cook, (Stf::RNG::Detail::SphereSamplers<float, 3>::rejection_cook));
MAKE_BENCH(sphere_3_spec_polar, (Stf::RNG::Detail::SphereSamplers<float, 3>::polar));
MAKE_BENCH(sphere_3_spec_polar_noinverse, (Stf::RNG::Detail::SphereSamplers<float, 3>::polar_noinverse));

MAKE_GENERAL_BENCH(sphere, gaussian, float, Stf::RNG::Detail::SphereSamplers, general_gaussian);

MAKE_BENCH(ball_1_spec_snorm, (Stf::RNG::Detail::BallSamplers<float, 1>::snorm));
MAKE_BENCH(ball_2_spec_rejection, (Stf::RNG::Detail::BallSamplers<float, 2>::rejection));
MAKE_BENCH(ball_2_spec_concentric, (Stf::RNG::Detail::BallSamplers<float, 2>::concentric));

MAKE_GENERAL_BENCH(ball, polar_radial, float, Stf::RNG::Detail::BallSamplers, polar_radial);
MAKE_GENERAL_BENCH(ball, rejection, float, Stf::RNG::Detail::BallSamplers, general_rejection);