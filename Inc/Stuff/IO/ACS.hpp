#pragma once

#include <functional>
#include <numeric>

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

struct ACS {
    Type type;
    float zero_point;
    float voltage_ceil = 3.3f;
    float voltage_division = 4096.f;

    size_t bump_counter = 0;
    std::array<float, 8> averaging_window {};
    size_t window_ptr = 0;

    float get_sensitivity() {
        switch (type) {
        case Type::B50:
            return 40.f;
        case Type::U50:
            return 60.f;
        case Type::B100:
            return 20.f;
        case Type::U100:
            return 40.f;
        case Type::B150:
            return 13.333f;
        case Type::U150:
            return 26.667f;
        case Type::B200:
            return 10.f;
        case Type::U200:
            return 20.f;
        }
    }

    constexpr float get_averaged_voltage() {
        const auto sum = std::reduce(
            averaging_window.cbegin(), averaging_window.cbegin() + std::min(averaging_window.size(), bump_counter));
        return sum / averaging_window.size();
    }

    constexpr void bump(float voltage) {
        averaging_window[window_ptr] = voltage * (voltage_ceil / voltage_division);
        ++window_ptr %= averaging_window.size();
        ++bump_counter;
    }

    float measure_amperes(bool use_zero_pt = false) {
        const auto measurement = get_averaged_voltage();

        float normalized;

        if (use_zero_pt)
            normalized = measurement - zero_point;
        else
            normalized = measurement - (3.3f / 2.f);

        const auto scaled = normalized * 1000.f / get_sensitivity();

        return scaled;
    }
};

}
