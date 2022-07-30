#pragma once

#include <cstdint>
#include <functional>
#include <numeric>
#include <utility>

namespace Stf::IO::ACS {

enum class Type : int {
    CET10,
    B50,
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
    float zero_point = 3.3f / 2.f;
    float voltage_ceil = 3.3f;
    float voltage_division = 4095.f;

    size_t bump_counter = 0;
    std::array<float, 8> averaging_window {};
    size_t window_ptr = 0;

    constexpr float get_sensitivity() const {
        switch (type) {
        case Type::CET10:
            return 40.f * 5.f;
        case Type::B50:
            return 40.f;
        case Type::U50:
            return 60.f;
        case Type::B100:
            return 20.f;
        case Type::U100:
            return 40.f;
        default: [[fallthrough]];
        case Type::B150:
            return 13.333f;
        case Type::U150:
            return 26.667f;
        case Type::B200:
            return 10.f;
        case Type::U200:
            return 20.f;
        }
        std::unreachable();
    }

    constexpr float get_averaged_voltage() const {
        const auto sum = std::reduce(
            averaging_window.cbegin(), averaging_window.cbegin() + std::min(averaging_window.size(), bump_counter));
        return sum / averaging_window.size();
    }

    constexpr void bump(float measurement) {
        averaging_window[window_ptr] = measurement;
        ++window_ptr %= averaging_window.size();
        ++bump_counter;
    }

    constexpr void bump(uint32_t measurement) {
        bump(static_cast<float>(measurement) * (voltage_ceil / voltage_division));
    }

    constexpr float measure_amperes(bool use_zero_pt = false) const {
        const auto measurement = get_averaged_voltage();

        float normalized;

        if (use_zero_pt)
            normalized = measurement - zero_point;
        else
            normalized = measurement - (voltage_ceil / 2.f);

        const auto scaled = normalized * 1000.f / get_sensitivity();

        return scaled;
    }
};

}
