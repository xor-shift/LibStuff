#pragma once

#include <functional>

namespace Stf::IO::ACS {

enum class Type : int {
    B50 = 0,
    U50,
    B100,
    U100,
    B150,
    U150,
    B200,
    U200,
};

template<typename Reader>
struct ACSHandle {
    Reader reader;
    Type type;
    float zero_point;

    float get_sensitivity() {
        switch (type) {
            case Type::B50: return 40.f;
            case Type::U50: return 60.f;
            case Type::B100: return 20.f;
            case Type::U100: return 40.f;
            case Type::B150: return 13.333f;
            case Type::U150: return 26.667f;
            case Type::B200: return 10.f;
            case Type::U200: return 20.f;
        }
    }

    float measure_single_raw() {
        return static_cast<float>(std::invoke(reader)) * (3.3f / 4095.f);
    }

    float averaged_measurement(size_t n_runs = 1) {
        if (n_runs <= 1)
            return measure_single_raw();

        float sum = 0.f;
        for (size_t i = 0; i < n_runs; i++)
            sum += measure_single_raw();

        return sum / static_cast<float>(n_runs);
    }

    template<bool use_zero_pt = false>
    float measure_amperes(size_t n_runs = 1) {
        const auto measurement = averaged_measurement(n_runs);

        float normalized;

        if constexpr (use_zero_pt)
            normalized = measurement - zero_point;
        else
            normalized = measurement - (3.3f / 2.f);

        const auto scaled = normalized * 1000.f / get_sensitivity();

        return scaled;
    }
};

}
