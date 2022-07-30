#pragma once

#include <numbers>
#include <numeric>

#include <Stuff/IO/GPS.hpp>
#include <Stuff/IO/Nextion.hpp>
#include <Stuff/Maths/Maths.hpp>
#include <Stuff/Refl/ReflNew.hpp>
#include <Stuff/Refl/Serde.hpp>

#include <Uncommon/LoRa.hpp>
#include <Uncommon/MPU9250.hpp>

#include "./CEPackets.hpp"

namespace Unc {

inline float get_timestamp() {
    return std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::system_clock::now().time_since_epoch()).count();
}

struct Interval {
    uint32_t interval;
    uint32_t offset = 0;
    uint32_t last = 0;

    constexpr bool operator()(uint32_t now) {
        now += offset;
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

template<size_t TxBufferSize, size_t RxBufferSize> struct Connection final : public Stf::PacketManagerBase<TxBufferSize, RxBufferSize> {
    void prepare_tx() override {
        uint8_t asd[] = { 0, 63, 31 };
        tx_buffer(asd, sizeof(asd));
    }

    // void tx_byte(uint8_t b) override { HAL_UART_Transmit(&Stf::uart_lora, &b, 1, 100); }

    void tx_buffer(uint8_t* b, size_t n) override { HAL_UART_Transmit(&Stf::uart_lora, b, n, 1000); }

    template<typename T> void dump_memory(uint32_t addr) {
        using U = decltype(std::declval<T>().data);
        volatile U* p = reinterpret_cast<volatile U*>(static_cast<std::uintptr_t>(addr));
        this->template tx_packet<T>(T {
            .address = addr,
            .data = *p,
        });
    }

    void rx_packet(Unc::MemoryRequestPacket packet, Stf::PacketHeader header) {
        switch (packet.size) {
        case 1: return dump_memory<MemoryResponsePacket8B>(packet.address);
        case 2: return dump_memory<MemoryResponsePacket16B>(packet.address);
        case 4: return dump_memory<MemoryResponsePacket32B>(packet.address);
        case 8: return dump_memory<MemoryResponsePacket64B>(packet.address);
        default: return;
        }
    }

    void rx_packet(std::span<const uint8_t> p, Stf::PacketHeader header) override {
#define PACKET_HANDLNG_CASE(type)                                \
    case type::packet_id: {                                      \
        const auto expected_size = Stf::serialized_size_v<type>; \
        if (header.len != expected_size) {                       \
            break;                                               \
        }                                                        \
                                                                 \
        auto packet = Stf::deserialize<type>(p.begin());         \
        rx_packet(std::move(packet), header);                    \
                                                                 \
        break;                                                   \
    }

        switch (header.id) {
            PACKET_HANDLNG_CASE(Unc::MemoryRequestPacket);
        default: break;
        }
    }
};

struct NewTelemetryState {
    static constexpr double k_battery_mah = 15000.f;

    /// VCS information
    float voltage_smps = 1.f;
    float current_smps = 2.f;
    float voltage_engine = 3.f;
    float current_engine = 4.f;
    float voltage_engine_driver = 5.f;
    float current_engine_driver = 6.f;
    float voltage_bms = 7.f;
    float current_bms = 8.f;
    float voltage_telemetry = 8.f;
    float current_telemetry = 9.f;
    float temperature_engine_driver = 10.f;
    float temperature_smps = 11.f;
    bool ac_status = false;

    /// BMS information
    std::array<float, 27> battery_voltages;
    std::array<float, 5> battery_temperatures;
    float spent_mah;
    float spent_mwh;
    float soc_percent;
    float current;

    /// Engine information
    float engine_rpm = 0.f;
    float reported_engine_duty = 0.f;

    /// Isolation information
    std::array<double, 2> isolation_resistances { 123456, 7890 };

    /// Auxiliary state
    Stf::Delim::ByteReader<76, 2> gps_reader;
    Stf::GPS::GPSState gps_state;

    Connection<512, 512> s_connection {};
    Lora lora {
        .huart = Stf::uart_lora,
        .aux_port = LORA_IN_AUX_GPIO_Port,
        .aux_pin = LORA_IN_AUX_Pin,
        .m0_port = LORA_OUT_M0_GPIO_Port,
        .m0_pin = LORA_OUT_M0_Pin,
        .m1_port = LORA_OUT_M1_GPIO_Port,
        .m1_pin = LORA_OUT_M1_Pin,
    };

    MPU9250 mpu { hi2c1,
        MPU9250::Configuration {
            .accel_self_tests = { false, false, false },
            .accel_full_scale = Unc::MPU9250::AccelFS::G2,
            .gyro_self_tests = { false, false, false },
            .gyro_full_scale = Unc::MPU9250::GyroFS::D250,
        } };

    CANHandle can_handle;

    /// Sensors, IO, etc.
    uint32_t adc_result = 0;

    // ADC-dependant sensors
    Stf::IO::ACS::ACS acs {
        .type = Stf::IO::ACS::Type::CET10,
        .zero_point = 0.f,
    };

    void init() {
        mpu.dumb_init();

        const auto pclk = HAL_RCC_GetPCLK1Freq();
        auto* const handle = lora.huart.Instance;
        const auto orig_baud = lora.huart.Init.BaudRate;

        handle->CR1 &= ~(USART_CR1_UE);
        handle->BRR = UART_BRR_SAMPLING16(pclk, 9600);
        handle->CR1 |= USART_CR1_UE;

        lora.set_mode(Unc::Lora::Mode::Configuration);
        const auto conf_res_0 = lora.read_configuration();
        const auto conf_res_1 = lora.configure(k_tx_config);
        const auto conf_res_2 = lora.read_configuration();
        lora.set_mode(Unc::Lora::Mode::Normal);

        handle->CR1 &= ~(USART_CR1_UE);
        handle->BRR = UART_BRR_SAMPLING16(pclk, orig_baud);
        handle->CR1 |= USART_CR1_UE;

        CAN_FilterTypeDef filter = {
            .FilterIdHigh = 0,
            .FilterIdLow = 0,
            .FilterMaskIdHigh = 0,
            .FilterMaskIdLow = 0,
            .FilterFIFOAssignment = CAN_RX_FIFO0,
            .FilterBank = 0,
            .FilterMode = CAN_FILTERMODE_IDMASK,
            .FilterScale = CAN_FILTERSCALE_32BIT,
            .FilterActivation = ENABLE,
            .SlaveStartFilterBank = 14,
        };

        if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
            Error_Handler();
        }

        can_handle = CANHandle(hcan1, static_cast<uint16_t>(41), CAN_RX_FIFO0);
        HAL_ADC_Start_IT(&hadc1);
    }

    constexpr void new_adc_result(uint32_t result) {
        adc_result = result;
        acs.bump(result);
    }

    void update_gps(char c) {
        if (gps_reader.update(c))
            return;

        const auto data = gps_reader.get_with_delimiter();
        gps_state.feed_line({ reinterpret_cast<const char*>(data.data()), data.size() });

        gps_reader.reset();
        gps_reader.update(c);
    }

    void lora_byte(uint8_t b) { s_connection.rx_byte(b); }

    constexpr float get_temperature() const {
        const auto v_cc = 3.3f;
        const auto adc_resolution = 1.f / 4095.f;

        const auto ntc_nominal = Stf::NTC::Nominal::R10K;
        const bool known_r_is_on_voltage_side = true;
        const float r_known = 10000.f;

        const auto v_m = static_cast<float>(adc_result) * adc_resolution * v_cc;
        const auto [r_1, r_2] = Stf::NTC::solve_voltage_divider(r_known, v_cc, v_m);

        return Stf::NTC::calculate_ntc(ntc_nominal, known_r_is_on_voltage_side ? r_2 : r_1);
    }

    constexpr float get_requested_duty() const {
        return adc_result * 100.f / 4096.f;
        // return 25.f;
    }

    constexpr float get_measured_current() const { return acs.measure_amperes(); }

    constexpr float get_speed() const {
        const float diameter_mm = 288.f;

        const float circumference = diameter_mm * 2 * std::numbers::pi_v<float>;
        const float mm_per_minute = circumference * engine_rpm;
        const float km_per_hour = mm_per_minute * 60.f / (1000.f * 1000.f);

        return km_per_hour;
    }

    constexpr void process_can_rx(uint16_t id, std::span<const uint8_t> data) {
        auto update_bms = [this, &data](size_t start_index, size_t len = 8) {
            for (size_t i = 0; i < len; i++) {
                auto& out = battery_voltages[i + start_index];
                out = Stf::map<float>(data[i], 0, 255, 2.4, 4.3);
            }
        };

        switch (static_cast<CANIDs>(id)) {
        case CANIDs::BMS1: update_bms(0, 8); break;
        case CANIDs::BMS2: update_bms(8, 8); break;
        case CANIDs::BMS3: update_bms(16, 8); break;
        case CANIDs::BMS4:
            update_bms(24, 3);
            for (size_t i = 0; i < 5; i++) {
                battery_temperatures[i] = Stf::map<float>(data[i + 3], 0, 255, 0, 100);
            }
            break;
        case CANIDs::BMS5: {
            uint16_t raw_mah = Stf::convert_endian(*reinterpret_cast<const uint16_t*>(data.data()), std::endian::big);
            uint16_t raw_mwh = Stf::convert_endian(*reinterpret_cast<const uint16_t*>(data.data() + 2), std::endian::big);
            uint16_t raw_current = Stf::convert_endian(*reinterpret_cast<const uint16_t*>(data.data() + 4), std::endian::big);
            uint16_t raw_percent = Stf::convert_endian(*reinterpret_cast<const uint16_t*>(data.data() + 6), std::endian::big);
            spent_mah = Stf::map<float>(raw_mah, 0, 65535, 0, 15000);
            spent_mwh = Stf::map<float>(raw_mwh, 0, 65535, 0, 15000 * 256);
            current = Stf::map<float>(raw_current, 0, 65535, -10, 50);
            soc_percent = Stf::map<float>(raw_percent, 0, 65535, -5, 105);
            volatile auto a = raw_mah + raw_mwh + raw_current;
        } break;
        case CANIDs::EngineRPMTemp: {
            engine_rpm = *reinterpret_cast<const float*>(data.data());
            reported_engine_duty = *reinterpret_cast<const float*>(data.data() + 4);
            volatile auto a = engine_rpm;
        } break;
        case CANIDs::VCSTemperatures: [[fallthrough]];
        case CANIDs::VCSEngine: [[fallthrough]];
        case CANIDs::VCSTelemetry: [[fallthrough]];
        case CANIDs::VCSSMPS: [[fallthrough]];
        case CANIDs::VCSBMS: {
            float f_0;
            float f_1;
            std::memcpy(&f_0, data.data(), 4);
            std::memcpy(&f_1, data.data() + 4, 4);

            const std::array<std::array<float NewTelemetryState::*, 2>, 5> targets = { {
                { &NewTelemetryState::temperature_smps, &NewTelemetryState::temperature_engine_driver },
                { &NewTelemetryState::voltage_engine_driver, &NewTelemetryState::current_engine_driver },
                { &NewTelemetryState::voltage_telemetry, &NewTelemetryState::current_telemetry },
                { &NewTelemetryState::voltage_smps, &NewTelemetryState::current_smps },
                { &NewTelemetryState::voltage_bms, &NewTelemetryState::current_bms },
            } };

            const auto offset = id - static_cast<uint16_t>(CANIDs::VCSTemperatures);

            const auto& [t_0, t_1] = targets[offset];
            this->*t_0 = f_0;
            this->*t_1 = f_1;
        } break;
        case CANIDs::VCSACStatus: {
            if (data[0] != 0 && data[0] != 255)
                break;
            ac_status = data[0] == 255;
            volatile auto a = ac_status;
        } break;
        case CANIDs::IsolationFirst: [[fallthrough]];
        case CANIDs::IsolationSecond: {
            const auto v = std::bit_cast<double>(data);
            const auto index = id - static_cast<uint16_t>(CANIDs::IsolationFirst);
            isolation_resistances[index] = Stf::convert_endian(v, std::endian::little);
        } break;

        default: break;
        }
    }

    void push_nextion() {
        using namespace std::string_view_literals;

        Stf::Nextion::set("volt_smps", voltage_smps, 1);
        Stf::Nextion::set("curr_smps", current_smps, 1);
        Stf::Nextion::set("volt_engine", voltage_engine, 1);
        Stf::Nextion::set("curr_engine", current_engine, 1);
        Stf::Nextion::set("volt_engine_driver", voltage_engine_driver, 1);
        Stf::Nextion::set("curr_engine_driver", current_engine_driver, 1);
        Stf::Nextion::set("volt_bms", voltage_bms, 1);
        Stf::Nextion::set("curr_bms", current_bms, 1);
        Stf::Nextion::set("volt_telemetry", voltage_telemetry, 1);
        Stf::Nextion::set("curr_telemetry", current_telemetry, 1);
        Stf::Nextion::set("temp_engine_drv", temperature_engine_driver, 1);
        Stf::Nextion::set("temp_smps", temperature_smps, 1);

        Stf::Nextion::set("cell_init", battery_voltages[0], 2);
        for (int i = 0; i < 26; i++) {
            char str[] = "cell_ ";
            str[5] = 'A' + i;
            Stf::Nextion::set(str, battery_voltages[i + 1], 2);
        }

        const auto sum = std::reduce(battery_voltages.cbegin(), battery_voltages.cend());
        const auto [min, max] = std::minmax_element(battery_voltages.cbegin(), battery_voltages.cend());
        const auto avg = sum / battery_voltages.size();
        Stf::Nextion::set("cell_min", *min, 2);
        Stf::Nextion::set("cell_max", *max, 2);
        Stf::Nextion::set("cell_avg", avg, 2);
        Stf::Nextion::set("cell_sum", sum, 2);

        // Stf::Nextion::set("temp_batt_max", *std::max_element(battery_temperatures.cbegin(), battery_temperatures.cend()), 1);
        Stf::Nextion::set("temp_batt_max", battery_temperatures[0], 1);
        for (int i = 0; i < 5; i++) {
            char str[] = "temp_batt_ ";
            str[10] = '0' + i;
            Stf::Nextion::set(str, battery_temperatures[i], 1);
        }

        Stf::Nextion::set("current", current, 1);
        Stf::Nextion::set("spent_wh", spent_mwh / 1000.f, 1);
        // Stf::Nextion::set("spent_ah", spent_mah / 1000.f, 1);
        // Stf::Nextion::set("spent_mwh", spent_mwh, 1);
        // Stf::Nextion::set("spent_mah", spent_mah, 1);
        Stf::Nextion::set("soc_percent", soc_percent, 1);
        // Stf::Nextion::set("soc_normal", soc_percent / 100.f, 1);

        Stf::Nextion::set("speed_kmh", static_cast<int>(get_speed()));
        Stf::Nextion::set("rpm_engine", engine_rpm, 1);

        Stf::Nextion::set("loop_count", 100.f, 1);
        Stf::Nextion::set("lap_count", 100.f, 1);
        // Stf::Nextion::set("lap_avg_time_left", "00:00:00"sv);
        // Stf::Nextion::set("eta_charge", "00:00:00"sv);
        // Stf::Nextion::set("performance_1s", 1);
        // Stf::Nextion::set("performance_5s", 5);
        // Stf::Nextion::set("performance_15s", 15);

        auto convert_to_si = [](double v, std::span<char> buf) -> size_t {
            std::array<std::pair<double, char>, 3> lookup { {
                { 1'000'000'000, 'G' },
                { 1'000'000, 'M' },
                { 1'000, 'K' },
            } };

            bool neg = v < 0;
            v = std::abs(v);

            char c;
            for (auto& [m, l] : lookup) {
                if (v >= m) {
                    c = l;
                    v /= m;
                    break;
                }
            }

            auto t = static_cast<int>(v);

            auto it = buf.data();

            auto push_digit = [&](int d) {
                if (d > 10)
                    d %= 10;
                if (d == 0 && it == buf.data())
                    return;

                const auto c = static_cast<char>(d) + '0';
                *it++ = c;
            };

            push_digit((t / 100) % 10);
            push_digit((t / 10) % 10);
            push_digit((t / 1) % 10);
            *it++ = c;

            return std::distance(buf.data(), it);
        };
        std::array<char, 64> isolation_buf;
        const auto len = convert_to_si(isolation_resistances[0], isolation_buf);
        const std::string_view isolation_sv(isolation_buf.data(), len);
        Stf::Nextion::set("isolation_res", isolation_sv);

        Stf::Nextion::set("ac_status", ac_status ? "sarj"sv : "surus"sv);

        if (const auto charge_current = Stf::abs(current); current > 0 && charge_current > .25) {
            double remaining_seconds = Stf::abs((k_battery_mah - spent_mah) / (current * 1000.f)) * 86400.;
            double remaining_hours = std::floor(remaining_seconds / 3600.);
            remaining_seconds -= remaining_hours * 3600.;
            double remaining_minutes = std::floor(remaining_seconds / 60.);
            remaining_seconds -= remaining_minutes * 60.;
            std::array<char, 8> eta_buf {};
            eta_buf[0] = static_cast<char>(static_cast<int>(remaining_hours / 10.) % 10) + '0';
            eta_buf[1] = static_cast<char>(static_cast<int>(remaining_hours / 1.) % 10) + '0';
            eta_buf[2] = ':';
            eta_buf[3] = static_cast<char>(static_cast<int>(remaining_minutes / 10.) % 10) + '0';
            eta_buf[4] = static_cast<char>(static_cast<int>(remaining_minutes / 1.) % 10) + '0';
            eta_buf[5] = ':';
            eta_buf[6] = static_cast<char>(static_cast<int>(remaining_seconds / 10.) % 10) + '0';
            eta_buf[7] = static_cast<char>(static_cast<int>(remaining_seconds / 1.) % 10) + '0';
            std::string_view eta_view(eta_buf.data(), 8);
            Stf::Nextion::set("eta_charge", eta_view);
        } else {
            Stf::Nextion::set("eta_charge", "sarj olmuyor"sv);
        }
    }

private:
    static constexpr Unc::E22Config k_tx_config = {
        .address = 64,
        .network_id = 0,

        .serial_rate = Unc::E22Config::SerialRate::BPS115200,
        .air_rate = Unc::E22Config::AirRate::BPS9600,
        .parity = Unc::E22Config::Parity::None,

        .sub_packet_setting = Unc::E22Config::SubPacket::B240,
        .rssi_ambient_noise_enable = false,
        .transmitting_power = Unc::E22Config::Power::DBM22,

        .channel = 31,

        .enable_rssi = false,
        .fixed_point_transmission = true,
        .repeater_function = false,
        .monitor_before_transmission = false,
        .wor_transmitter = false,
        .wor_cycle = 3,

        .encryption_key = 0,
    };
};

VCSPacket VCSPacket::from_state(NewTelemetryState const& state) {
    return {
        .timestamp = get_timestamp(),
        .voltage_smps = state.voltage_smps,
        .current_smps = state.current_smps,
        .voltage_engine = state.voltage_engine,
        .current_engine = state.current_engine,
        .voltage_engine_driver = state.voltage_engine_driver,
        .current_engine_driver = state.current_engine_driver,
        .voltage_bms = state.voltage_bms,
        .current_bms = state.current_bms,
        .voltage_telemetry = state.voltage_telemetry,
        .current_telemetry = state.current_telemetry,
        .temperature_engine_driver = state.temperature_engine_driver,
        .temperature_smps = state.temperature_smps,
    };
}

BMSSummaryPacket BMSSummaryPacket::from_state(NewTelemetryState const& state) {
    const auto sum = std::reduce(state.battery_voltages.cbegin(), state.battery_voltages.cend());
    const auto [min, max] = std::minmax_element(state.battery_voltages.cbegin(), state.battery_voltages.cend());
    const auto avg = sum / state.battery_voltages.size();

    return {
        .timestamp = get_timestamp(),
        .min_max_avg_cell_voltages { *min, *max, avg },
        .cell_sum = sum,
        .battery_temperatures = state.battery_temperatures,
        .spent_mah = state.spent_mah,
        .spent_mwh = state.spent_mwh,
        .soc_percent = state.soc_percent,
        .current = state.current,
    };
}

EnginePacket EnginePacket::from_state(NewTelemetryState const& state) {
    return {
        .timestamp = get_timestamp(),
        .speed = state.get_speed(),
        .rpm = state.engine_rpm,
        .user_duty = state.get_requested_duty(),
        .reported_duty = state.reported_engine_duty,
    };
}

LocalObservationsPacket LocalObservationsPacket::from_state(NewTelemetryState const& state) {
    auto [lat, lon] = state.gps_state.location.value_or(std::pair { Stf::GPS::Angle {}, Stf::GPS::Angle {} });
    return {
        .timestamp = get_timestamp(),
        .gps_coords = { lat.as_degrees(), lon.as_degrees() },
        .acceleration = state.mpu.get_accel(),
        .orientation = state.mpu.get_gyro(),
    };
}

BatteryArrayPacket BatteryArrayPacket::from_state(NewTelemetryState const& state) {
    return {
        .timestamp = get_timestamp(),
        .voltages = state.battery_voltages,
    };
}

EssentialsPacket EssentialsPacket::from_state(NewTelemetryState const& state) {
    return {
        .timestamp = get_timestamp(),
        .speed = state.get_speed(),
        .max_temp = *std::max_element(state.battery_temperatures.cbegin(), state.battery_temperatures.cend()),
        .cell_sum = std::reduce(state.battery_voltages.cbegin(), state.battery_voltages.cend()),
        .spent_wh = state.spent_mwh / 1000.f,
        .spent_ah = state.spent_mah / 1000.f,
    };
}

}
