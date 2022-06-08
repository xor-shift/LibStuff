#pragma once

// no includes, this is not meant to be used standalone

namespace Unc {

struct E22Config {
    enum class SerialRate : uint8_t {
        BPS1200 = 0,
        BPS2400 = 1,
        BPS4800 = 2,
        BPS9600 = 3,
        BPS19200 = 4,
        BPS38400 = 5,
        BPS57600 = 6,
        BPS115200 = 7,
    };

    enum class AirRate : uint8_t {
        BPS2400 = 0,
        BPS4800 = 3,
        BPS9600 = 4,
        BPS19200 = 5,
        BPS38400 = 6,
        BPS62500 = 7,
    };

    enum class Parity : uint8_t {
        None = 0,
        Odd = 1,
        Even = 2,
    };

    enum class SubPacket : uint8_t {
        B240 = 0,
        B128 = 1,
        B64 = 2,
        B32 = 3,
    };

    enum class Power : uint8_t {
        DBM22 = 0,
        DBM17 = 1,
        DBM13 = 2,
        DBM10 = 3,
    };

    uint16_t address = 0;
    uint8_t network_id = 0;

    SerialRate serial_rate = SerialRate::BPS9600;
    AirRate air_rate = AirRate::BPS2400;
    Parity parity = Parity::None;

    SubPacket sub_packet_setting = SubPacket::B240;
    bool rssi_ambient_noise_enable = false;
    Power transmitting_power = Power::DBM22;

    uint8_t channel = 0;

    bool enable_rssi = false;
    bool fixed_point_transmission = false;
    bool repeater_function = false;
    bool monitor_before_transmission = false;
    bool wor_transmitter = false;
    // WOR cycle will be (wor_cycle + 1) * 500ms
    uint8_t wor_cycle = 3;

    uint16_t encryption_key = 0;

    static constexpr E22Config disassemble(std::span<const uint8_t, 7> regs) {
        auto shift_and_cast = []<typename T, typename U>(T& target, U v, auto shift, auto mask) {
            const auto s = static_cast<U>(shift);
            const auto m = static_cast<U>(mask);
            target = static_cast<T>((v >> s) & m);
        };

        E22Config config {
            .address = static_cast<uint16_t>((static_cast<uint16_t>(regs[0]) << 8) | regs[1]),
            .network_id = regs[2],
            .channel = regs[5],
        };

        shift_and_cast(config.serial_rate, regs[3], 5, 0x07);
        uint8_t parity;
        shift_and_cast(parity, regs[3], 3, 0x03);
        config.parity = static_cast<Parity>(parity == 3 ? 0 : parity);
        shift_and_cast(config.air_rate, regs[3], 0, 0x07);

        shift_and_cast(config.sub_packet_setting, regs[4], 6, 0x03);
        shift_and_cast(config.rssi_ambient_noise_enable, regs[4], 5, 0x01);
        shift_and_cast(config.transmitting_power, regs[4], 0, 0x03);

        shift_and_cast(config.enable_rssi, regs[6], 7, 1);
        shift_and_cast(config.fixed_point_transmission, regs[6], 6, 1);
        shift_and_cast(config.repeater_function, regs[6], 5, 1);
        shift_and_cast(config.monitor_before_transmission, regs[6], 4, 1);
        shift_and_cast(config.wor_transmitter, regs[6], 3, 1);
        shift_and_cast(config.wor_cycle, regs[6], 0, 0x07);

        return config;
    }

    constexpr std::array<uint8_t, 9> assemble() const {
        std::array<uint8_t, 9> ret {};

        ret[0] = static_cast<uint8_t>(address >> 8);
        ret[1] = static_cast<uint8_t>(address & 0xFF);

        ret[2] = network_id;

        ret[3] |= static_cast<uint8_t>(serial_rate) << 5;
        ret[3] |= static_cast<uint8_t>(parity) << 3;
        ret[3] |= static_cast<uint8_t>(air_rate);

        ret[4] |= static_cast<uint8_t>(sub_packet_setting) << 6;
        ret[4] |= static_cast<uint8_t>(rssi_ambient_noise_enable) << 5;
        ret[4] |= static_cast<uint8_t>(transmitting_power);
        ret[4] &= 0b1110'0011;

        ret[5] = Stf::clamp<uint8_t>(channel, 0u, 80u);

        ret[6] |= static_cast<uint8_t>(enable_rssi) << 7;
        ret[6] |= static_cast<uint8_t>(fixed_point_transmission) << 6;
        ret[6] |= static_cast<uint8_t>(repeater_function) << 5;
        ret[6] |= static_cast<uint8_t>(monitor_before_transmission) << 4;
        ret[6] |= static_cast<uint8_t>(wor_transmitter) << 3;
        ret[6] |= Stf::clamp<uint8_t>(wor_cycle, 0, 7);

        ret[7] = static_cast<uint8_t>(encryption_key >> 8);
        ret[8] = static_cast<uint8_t>(encryption_key & 0xFF);

        return ret;
    }
};

struct Lora {
    enum class Mode {
        Normal = 0,
        WOR = 1,
        Configuration = 2,
        DeepSleep = 3,
    };

    UART_HandleTypeDef& huart;
    GPIO_TypeDef* aux_port;
    uint16_t aux_pin;
    GPIO_TypeDef* m0_port;
    uint16_t m0_pin;
    GPIO_TypeDef* m1_port;
    uint16_t m1_pin;

    Mode mode = Mode::Normal;

    struct ModeGuard {
        explicit ModeGuard(Lora& lora, Mode mode) noexcept
            : m_lora(lora)
            , m_new_mode(mode)
            , m_prev_mode(lora.mode) {
            m_lora.set_mode(mode);
        }

        ModeGuard(ModeGuard const&) = delete;
        ModeGuard(ModeGuard const&&) = delete;

        ModeGuard(ModeGuard&& other)
            : m_lora(other.m_lora)
            , m_new_mode(other.m_new_mode)
            , m_prev_mode(other.m_prev_mode) {
            other.armed = false;
        }

        ~ModeGuard() noexcept {
            if (!armed || m_new_mode == m_prev_mode)
                return;

            m_lora.set_mode(m_prev_mode);
        }

    private:
        bool armed = true;
        Lora& m_lora;
        Mode m_new_mode;
        Mode m_prev_mode;
    };

    void wait_aux(GPIO_PinState state = GPIO_PIN_SET) const {
        for (;;) {
            bool res = HAL_GPIO_ReadPin(aux_port, aux_pin) == state;
            if (res)
                break;
        }
        HAL_Delay(3);
    }

    void set_mode(Mode new_mode) {
        mode = new_mode;

        const int i_mode = static_cast<int>(mode);
        bool m0 = (i_mode & 0b01) != 0;
        bool m1 = (i_mode & 0b10) != 0;

        wait_aux();
        HAL_GPIO_WritePin(m0_port, m0_pin, m0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(m1_port, m1_pin, m1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
        wait_aux();
    }

    ModeGuard guarded_set_mode(Mode mode) { return ModeGuard(*this, mode); }

    enum class Command: uint8_t {
        SetRegisters = 0xC0,
        ReadRegisters = 0xC1,
        TemporarySetRegisters = 0xC2,
        WirelessConfiguration = 0xCF,
    };

    int read_registers(auto out_it, uint8_t address, uint8_t size, bool request = true) {
        std::array<uint8_t, 3> expected_command { 0xC1, address, size };
        std::array<uint8_t, 3> received_command {};
        std::array<uint8_t, 16> recv_buf {};

        if (request && (HAL_UART_Transmit(&huart, expected_command.data(), expected_command.size(), 30) != HAL_OK))
            return 1;

        if (HAL_UART_Receive(&huart, received_command.data(), received_command.size(), 30) != HAL_OK)
            return 2;

        if (expected_command != received_command)
            return 3;

        if (HAL_UART_Receive(&huart, recv_buf.data(), size, 140) != HAL_OK)
            return 4;

        std::copy_n(recv_buf.cbegin(), size, out_it);
        return 0;
    }

    std::optional<E22Config> read_configuration() {
        const auto guard = guarded_set_mode(Mode::Configuration);

        std::array<uint8_t, 7> read_buf {};
        if (read_registers(read_buf.begin(), 0, 7, true))
            return std::nullopt;

        return E22Config::disassemble(std::span<const uint8_t, 7>(read_buf.data(), 7));
    }

    std::optional<E22Config> configure(E22Config config) {
        const auto guard = guarded_set_mode(Mode::Configuration);

        std::array<uint8_t, 3> command_buf { 0xC0, 0x00, 0x07 };

        auto assembled = config.assemble();

        if (HAL_UART_Transmit(&huart, command_buf.data(), command_buf.size(), 30) != HAL_OK)
            return std::nullopt;

        if (HAL_UART_Transmit(&huart, assembled.data(), 7, 60) != HAL_OK)
            return std::nullopt;

        std::array<uint8_t, 7> read_buf {};
        if (read_registers(read_buf.begin(), 0, 7, false))
            return std::nullopt;

        return E22Config::disassemble(std::span<const uint8_t, 7>(read_buf.data(), 7));
    }
};

}