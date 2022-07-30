#pragma once

#include <Stuff/Refl/ReflNew.hpp>

namespace Unc {

class NewTelemetryState;

struct VCSPacket {
    static constexpr uint16_t packet_id = 1;

    MEMREFL_BEGIN(VCSPacket, 13)

    float MEMREFL_DECL_MEMBER(timestamp);

    float MEMREFL_DECL_MEMBER(voltage_smps);
    float MEMREFL_DECL_MEMBER(current_smps);
    float MEMREFL_DECL_MEMBER(voltage_engine);
    float MEMREFL_DECL_MEMBER(current_engine);
    float MEMREFL_DECL_MEMBER(voltage_engine_driver);
    float MEMREFL_DECL_MEMBER(current_engine_driver);
    float MEMREFL_DECL_MEMBER(voltage_bms);
    float MEMREFL_DECL_MEMBER(current_bms);
    float MEMREFL_DECL_MEMBER(voltage_telemetry);
    float MEMREFL_DECL_MEMBER(current_telemetry);
    float MEMREFL_DECL_MEMBER(temperature_engine_driver);
    float MEMREFL_DECL_MEMBER(temperature_smps);

    static VCSPacket from_state(NewTelemetryState const& state);
};

struct BMSSummaryPacket {
    static constexpr uint16_t packet_id = 2;

    MEMREFL_BEGIN(BMSSummaryPacket, 8);

    float MEMREFL_DECL_MEMBER(timestamp);
    std::array<float, 3> MEMREFL_DECL_MEMBER(min_max_avg_cell_voltages);
    float MEMREFL_DECL_MEMBER(cell_sum);
    std::array<float, 5> MEMREFL_DECL_MEMBER(battery_temperatures);
    float MEMREFL_DECL_MEMBER(spent_mah);
    float MEMREFL_DECL_MEMBER(spent_mwh);
    float MEMREFL_DECL_MEMBER(soc_percent);
    float MEMREFL_DECL_MEMBER(current);

    static BMSSummaryPacket from_state(NewTelemetryState const& state);
};

struct EnginePacket {
    static constexpr uint16_t packet_id = 3;

    MEMREFL_BEGIN(EnginePacket, 5);

    float MEMREFL_DECL_MEMBER(timestamp);
    float MEMREFL_DECL_MEMBER(speed);
    float MEMREFL_DECL_MEMBER(rpm);
    float MEMREFL_DECL_MEMBER(user_duty);
    float MEMREFL_DECL_MEMBER(reported_duty);

    static EnginePacket from_state(NewTelemetryState const& state);
};

struct LocalObservationsPacket {
    static constexpr uint16_t packet_id = 4;

    MEMREFL_BEGIN(LocalObservationsPacket, 4);

    float MEMREFL_DECL_MEMBER(timestamp);
    std::pair<float, float> MEMREFL_DECL_MEMBER(gps_coords);
    Stf::Vector<float, 3> MEMREFL_DECL_MEMBER(acceleration);
    Stf::Vector<float, 3> MEMREFL_DECL_MEMBER(orientation);

    static LocalObservationsPacket from_state(NewTelemetryState const& state);
};

struct BatteryArrayPacket {
    static constexpr uint16_t packet_id = 5;

    MEMREFL_BEGIN(BatteryArrayPacket, 2)
    float MEMREFL_DECL_MEMBER(timestamp);
    std::array<float, 27> MEMREFL_DECL_MEMBER(voltages);

    static BatteryArrayPacket from_state(NewTelemetryState const& state);
};

struct MemoryRequestPacket {
    static constexpr uint16_t packet_id = 6;

    MEMREFL_BEGIN(MemoryRequestPacket, 2);

    uint32_t MEMREFL_DECL_MEMBER(address);
    //8, 16, 32, 64
    uint8_t MEMREFL_DECL_MEMBER(size);
};

struct MemoryResponsePacket8B {
    static constexpr uint16_t packet_id = 7;

    MEMREFL_BEGIN(MemoryResponsePacket8B, 2);
    uint32_t MEMREFL_DECL_MEMBER(address);
    uint8_t MEMREFL_DECL_MEMBER(data);
};

struct MemoryResponsePacket16B {
    static constexpr uint16_t packet_id = 8;

    MEMREFL_BEGIN(MemoryResponsePacket16B, 2);
    uint32_t MEMREFL_DECL_MEMBER(address);
    uint16_t MEMREFL_DECL_MEMBER(data);
};

struct MemoryResponsePacket32B {
    static constexpr uint16_t packet_id = 9;

    MEMREFL_BEGIN(MemoryResponsePacket32B, 2);
    uint32_t MEMREFL_DECL_MEMBER(address);
    uint32_t MEMREFL_DECL_MEMBER(data);
};

struct MemoryResponsePacket64B {
    static constexpr uint16_t packet_id = 10;

    MEMREFL_BEGIN(MemoryResponsePacket64B, 2);
    uint32_t MEMREFL_DECL_MEMBER(address);
    uint64_t MEMREFL_DECL_MEMBER(data);
};

struct EssentialsPacket {
    static constexpr uint16_t packet_id = 11;

    MEMREFL_BEGIN(EssentialsPacket, 6);

    float MEMREFL_DECL_MEMBER(timestamp);
    float MEMREFL_DECL_MEMBER(speed);
    float MEMREFL_DECL_MEMBER(max_temp);
    float MEMREFL_DECL_MEMBER(cell_sum);
    float MEMREFL_DECL_MEMBER(spent_wh);
    float MEMREFL_DECL_MEMBER(spent_ah);

    static EssentialsPacket from_state(NewTelemetryState const& state);

};

struct DebugMetricsPacket {
    static constexpr uint16_t packet_id = 0xFFFE;

    MEMREFL_BEGIN(DebugMetricsPacket, 2)

    float MEMREFL_DECL_MEMBER(timestamp);
    std::array<uint32_t, 3> MEMREFL_DECL_MEMBER(performance_report);
};

}