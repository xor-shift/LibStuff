#pragma once

#include <Stuff/IO/GPS.hpp>
#include <Stuff/Maths/Maths.hpp>
#include <Stuff/Refl/Refl.hpp>
#include <Stuff/Refl/Serde.hpp>

namespace Unc {

struct Interval {
    uint32_t interval;
    uint32_t offset = 0;
    uint32_t last = 0;

    constexpr bool operator()(uint32_t now) {
        const float delta = now - last;

        if (delta < interval)
            return false;

        last = now;

        return true;
    }
};

struct PerformanceTracker {
    std::array<uint32_t, 3> counters {};
    std::array<uint32_t, 3> reports {};

    std::array<Unc::Interval, 3> intervals { { { 1000 }, { 5000 }, { 15000 } } };

    void new_tick(uint32_t seconds) {
        for (size_t i = 0; i < 3; i++) {
            ++counters[i];

            if (intervals[i](seconds)) {
                reports[i] = counters[i];
                counters[i] = 0;
            }
        }
    }
};

// mock data to transform the passed secodns with to feed into Stuff::sin
struct MockProp {
    std::pair<float, float> offset;
    std::pair<float, float> scale;

    static constexpr MockProp from_range(float from, float to, float seconds = 1.f) {
        return {
            .offset { 0.f, (to + from) / 2.f },
            .scale { 1. / seconds, (to - from) / 2.f },
        };
    }

    float operator()(float seconds) const {
        const auto x = seconds * scale.first + offset.first;
        return std::sin(x) * scale.second + offset.second;
    }
};

struct TelemetryState {
    static constexpr float wh_capacity = 1000.f;

    // BMS data
    static constexpr MockProp mock_battery_voltages = MockProp::from_range(2.5f, 3.7f);
    std::array<float, 27> battery_voltages {};
    static constexpr MockProp mock_battery_temperatures = MockProp::from_range(30.f, 40.f, 30.f);
    std::array<float, 27> battery_temperatures {};
    static constexpr MockProp mock_spent_wh = MockProp::from_range(0.f, 500.f, 120.f);
    float spent_wh = 0.f;
    static constexpr MockProp mock_current = MockProp::from_range(0.f, 500.f, 60.f);
    float current = 0.f;

    // engine data
    static constexpr MockProp mock_speed = MockProp::from_range(0.f, 120.f, 30.f);
    float speed = 0.f;
    static constexpr MockProp mock_engine_temp = MockProp::from_range(40.f, 50.f, 15.f);
    float engine_temp = 0.f;
    static constexpr MockProp mock_engine_voltage = MockProp::from_range(110.f, 130.f, 60.f);
    float engine_voltage = 0.f;
    static constexpr MockProp mock_engine_current = MockProp::from_range(0.f, 20.f, 30.f);
    float engine_current = 0.f;

    // static constexpr MockProp mock_ = MockProp::from_range();

    // Locally observed data
    Stf::GPS::GPSState gps_state {};

    static constexpr std::array<float, 3> min_max_avg(auto range) {
        const auto sum = std::accumulate(range.cbegin(), range.cend(), 0.f);
        const auto avg = sum / static_cast<float>(range.size());
        const auto [min, max] = std::ranges::minmax(range);

        return { min, max, avg };
    }

    constexpr std::array<float, 3> min_max_avg_voltages() const { return min_max_avg(battery_voltages); }

    constexpr std::array<float, 3> min_max_avg_temperatures() const { return min_max_avg(battery_temperatures); }

    void mock_tick(float seconds) {
        for (size_t i = 0; i < battery_voltages.size(); i++) {
            battery_voltages[i] = mock_battery_voltages(seconds);
            battery_temperatures[i] = mock_battery_temperatures(seconds);
        }

        spent_wh = mock_spent_wh(seconds);
        current = mock_current(seconds);

        speed = mock_speed(seconds);
        engine_temp = mock_engine_temp(seconds);
        engine_voltage = mock_engine_voltage(seconds);
        engine_current = mock_engine_current(seconds);
    }

    void nextion_bump_bms_core() const {
        const auto [low_v, high_v, avg_v] = min_max_avg_voltages();
        const auto [low_t, high_t, avg_t] = min_max_avg_temperatures();

        const auto percent_charge = (spent_wh / wh_capacity) * 100.f;

        Stf::Nextion::set("v_high", high_v, 2);
        Stf::Nextion::set("v_low", low_v, 2);

        Stf::Nextion::set("volt", avg_v, 1);
        Stf::Nextion::set("v_avg", avg_v, 1);
        Stf::Nextion::set("t_avg", avg_t, 1);

        Stf::Nextion::set("watth", spent_wh, 1);
        Stf::Nextion::set("curr", current, 1);
        Stf::Nextion::set("soc", percent_charge, 2);
    }

    void nextion_bump_bms_arrays() const {
        for (int i = 0; i < 10; i++) {
            const char b1[4] = { 'c', '1', static_cast<char>('0' + i), '\0' };
            Stf::Nextion::set(b1, battery_voltages[9 + i], 2);
        }
    }

    void nextion_bump_engine() const {
        Stf::Nextion::set("spd", speed, 1);
        Stf::Nextion::set("msicaklik", engine_temp, 1);
        Stf::Nextion::set("mvoltaj", engine_voltage, 1);
        Stf::Nextion::set("makim", engine_current, 1);
    }
};

struct BMSSummaryPacket {
    static constexpr uint16_t packet_id = 1;

    MEMREFL_BEGIN(BMSSummaryPacket, 5);

    uint32_t MEMREFL_DECL_MEMBER(timestamp);
    std::array<float, 3> MEMREFL_DECL_MEMBER(min_max_avg_battery_voltage);
    std::array<float, 3> MEMREFL_DECL_MEMBER(min_max_avg_battery_temperature);
    float MEMREFL_DECL_MEMBER(spent_wh);
    float MEMREFL_DECL_MEMBER(current);
};

struct EngineSummaryPacket {
    static constexpr uint16_t packet_id = 2;

    MEMREFL_BEGIN(EngineSummaryPacket, 5);

    uint32_t MEMREFL_DECL_MEMBER(timestamp);
    float MEMREFL_DECL_MEMBER(speed);
    float MEMREFL_DECL_MEMBER(engine_temp);
    float MEMREFL_DECL_MEMBER(engine_voltage);
    float MEMREFL_DECL_MEMBER(engine_current);
};

struct LocalObservationsPacket {
    static constexpr uint16_t packet_id = 3;

    MEMREFL_BEGIN(LocalObservationsPacket, 5);

    uint32_t MEMREFL_DECL_MEMBER(timestamp);
    std::pair<float, float> MEMREFL_DECL_MEMBER(gps_coords);
    Stf::Vector<float, 3> MEMREFL_DECL_MEMBER(acceleration);
    Stf::Vector<float, 3> MEMREFL_DECL_MEMBER(gyro);
    float MEMREFL_DECL_MEMBER(temperature);
};

struct BatteryArraysPacket {
    static constexpr uint16_t packet_id = 4;

    MEMREFL_BEGIN(BatteryArraysPacket, 7)

    float MEMREFL_DECL_MEMBER(timestamp);

    float MEMREFL_DECL_MEMBER(min_v);
    float MEMREFL_DECL_MEMBER(max_v);

    std::array<uint8_t, 27> MEMREFL_DECL_MEMBER(voltages);

    float MEMREFL_DECL_MEMBER(min_t);
    float MEMREFL_DECL_MEMBER(max_t);

    std::array<uint8_t, 27> MEMREFL_DECL_MEMBER(temperatures);

    static constexpr BatteryArraysPacket from_state(float seconds, TelemetryState const& state) {
        const auto [low_v, high_v, avg_v] = state.min_max_avg_voltages();
        const auto [low_t, high_t, avg_t] = state.min_max_avg_temperatures();

        BatteryArraysPacket ret {
            .timestamp = seconds,
            .min_v = low_v,
            .max_v = high_v,
            .voltages = {},
            .min_t = low_t,
            .max_t = high_t,
            .temperatures = {},
        };

        for (size_t i = 0; i < 27; i++) {
            const auto param_v = Stf::inv_lerp(state.battery_voltages[i], low_v, high_v);
            const auto val_v = Stf::clamp(param_v * 255.f, 0.f, 255.f);
            const auto param_t = Stf::inv_lerp(state.battery_temperatures[i], low_t, high_t);
            const auto val_t = Stf::clamp(param_t * 255.f, 0.f, 255.f);

            ret.voltages[i] = static_cast<uint8_t>(val_v);
            ret.temperatures[i] = static_cast<uint8_t>(val_t);
        }

        return ret;
    }
};

struct DebugMetricsPacket {
    static constexpr uint16_t packet_id = 0xFFFE;

    MEMREFL_BEGIN(DebugMetricsPacket, 3)

    uint32_t MEMREFL_DECL_MEMBER(timestamp);
    std::array<uint32_t, 3> MEMREFL_DECL_MEMBER(performance_report);
    std::array<char, 77> MEMREFL_DECL_MEMBER(gps_last_received_line);
};

}
