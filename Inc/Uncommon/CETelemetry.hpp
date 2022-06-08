#pragma once

#include <Stuff/IO/GPS.hpp>
#include <Stuff/Maths/Maths.hpp>
#include <Stuff/Refl/Refl.hpp>
#include <Stuff/Refl/Serde.hpp>

namespace Unc {

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

    // health data
    bool bms_master = false;
    bool bms_slave = false;
    bool engine = false;

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

        return {min, max, avg};
    }

    constexpr std::array<float, 3> min_max_avg_voltages() const {
        return min_max_avg(battery_voltages);
    }

    constexpr std::array<float, 3> min_max_avg_temperatures() const {
        return min_max_avg(battery_temperatures);
    }

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

    void nextion_bump_health() const { }

    void nextion_bump_bms_core() const {
        const auto [low_v, high_v, avg_v] = min_max_avg_voltages();
        const auto [low_t, high_t, avg_t] = min_max_avg_temperatures();

        const auto percent_charge = (spent_wh / wh_capacity) * 100.f;

        Stf::Nextion::set("yuksekhucre", high_v, 2);
        Stf::Nextion::set("dusukhucre", low_v, 2);

        Stf::Nextion::set("voltaj", avg_v, 1);
        Stf::Nextion::set("ortalama", avg_v, 1);
        Stf::Nextion::set("bsicaklik", avg_t, 1);

        Stf::Nextion::set("watthour", spent_wh, 1);
        Stf::Nextion::set("akim", current, 1);
        Stf::Nextion::set("soh", percent_charge, 2);
    }

    void nextion_bump_bms_arrays() const {
        for (int i = 0; i < 10; i++) {
            const char b1[4] = { 'c', '1', static_cast<char>('0' + i), '\0' };
            Stf::Nextion::set(b1, battery_voltages[9 + i], 2);
        }
    }

    void nextion_bump_engine() const {
        Stf::Nextion::set("hiz", speed, 1);
        Stf::Nextion::set("msicaklik", engine_temp, 1);
        Stf::Nextion::set("mvoltaj", engine_voltage, 1);
        Stf::Nextion::set("makim", engine_current, 1);
    }
};

struct TelemetryPacket {
    static constexpr uint16_t packet_id = 0;

    MEMREFL_BEGIN(TelemetryPacket, 16)

    float MEMREFL_DECL_MEMBER(timestamp);

    // health data
    bool MEMREFL_DECL_MEMBER(bms_master);
    bool MEMREFL_DECL_MEMBER(bms_slave);
    bool MEMREFL_DECL_MEMBER(engine);

    // BMS data
    std::array<float, 3> MEMREFL_DECL_MEMBER(min_max_avg_battery_voltage);
    std::array<float, 3> MEMREFL_DECL_MEMBER(min_max_avg_battery_temperature);
    float MEMREFL_DECL_MEMBER(spent_wh);
    float MEMREFL_DECL_MEMBER(current);

    // engine data
    float MEMREFL_DECL_MEMBER(speed);
    float MEMREFL_DECL_MEMBER(engine_temp);
    float MEMREFL_DECL_MEMBER(engine_voltage);
    float MEMREFL_DECL_MEMBER(engine_current);

    // locally observed data
    std::pair<float, float> MEMREFL_DECL_MEMBER(gps_coords);
    Stf::Vector<float, 3> MEMREFL_DECL_MEMBER(acceleration);
    Stf::Vector<float, 3> MEMREFL_DECL_MEMBER(gyro);
    float MEMREFL_DECL_MEMBER(temperature);

    static constexpr TelemetryPacket from_state(float seconds, TelemetryState const& state) {
        TelemetryPacket ret {
            .timestamp = seconds,

            .bms_master = state.bms_master,
            .bms_slave = state.bms_slave,
            .engine = state.engine,

            .min_max_avg_battery_voltage = state.min_max_avg_voltages(),
            .min_max_avg_battery_temperature = state.min_max_avg_temperatures(),
            .spent_wh = state.spent_wh,
            .current = state.current,

            .speed = state.speed,
            .engine_temp = state.engine_temp,
            .engine_voltage = state.engine_voltage,
            .engine_current = state.engine_current,

            .gps_coords = {0.f, 0.f},
        };

        if (state.gps_state.location) {
            ret.gps_coords.first = state.gps_state.location->first.as_degrees();
            ret.gps_coords.second = state.gps_state.location->second.as_degrees();
        }

        return ret;
    }
};

struct BatteryArraysPacket {
    static constexpr uint16_t packet_id = 1;

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
    static constexpr uint16_t packet_id = 2;

    MEMREFL_BEGIN(DebugMetricsPacket, 2)

    std::array<uint32_t, 3> MEMREFL_DECL_MEMBER(performance_report);
    std::array<char, 77> MEMREFL_DECL_MEMBER(gps_last_received_line);
};

}
