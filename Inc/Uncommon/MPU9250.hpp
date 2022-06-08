#pragma once

// no includes, this is not meant to be used standalone

namespace Unc {

struct I2CAddress {
    I2C_HandleTypeDef &i2c_handle;
    uint16_t dev_addr;
    uint16_t mem_addr;

    template<typename T> void read(T &v) const {
        auto *const out = reinterpret_cast<uint8_t *>(&v);
        HAL_I2C_Mem_Read(&i2c_handle, dev_addr, mem_addr, 1, out, sizeof(T), 100);
    }

    template<typename T> void write(T const &v) const {
        HAL_I2C_Mem_Write(&i2c_handle, dev_addr, mem_addr, 1, &v, sizeof(T), 100);
    }

    I2CAddress const &operator=(uint8_t b) const {
        HAL_I2C_Mem_Write(&i2c_handle, dev_addr, mem_addr, 1, &b, 1, 100);
        return *this;
    }

    operator uint8_t() const {
        uint8_t b = 0;
        HAL_I2C_Mem_Read(&i2c_handle, dev_addr, mem_addr, 1, &b, 1, 100);
        return b;
    }
};

struct MPU9250 {
    static constexpr uint16_t device_address_lo = 0b1100'0000;
    static constexpr uint16_t device_address_hi = 0b1101'0000;

    enum class Register : uint16_t {
        SMPLRT_DIV = 0x19,
        CONFIG = 0x1A,
        GYRO_CONFIG = 0x1B,
        ACCEL_CONFIG_1 = 0x1C,
        ACCEL_CONFIG_2 = 0x1D,
        ACCEL_XOUT_H = 0x3B,
        ACCEL_XOUT_L = 0x3C,
        ACCEL_YOUT_H = 0x3D,
        ACCEL_YOUT_L = 0x3E,
        ACCEL_ZOUT_H = 0x40,
        ACCEL_ZOUT_L = 0x41,
        GYRO_XOUT_H = 0x43,
        GYRO_XOUT_L = 0x44,
        GYRO_YOUT_H = 0x45,
        GYRO_YOUT_L = 0x46,
        GYRO_ZOUT_H = 0x47,
        GYRO_ZOUT_L = 0x48,
        PWR_MGMT_1 = 0x6B,
        PWR_MGMT_2 = 0x6C,
        WHO_AM_I = 0x75,
    };

    enum class SampleRate : uint8_t {
        SR_8000 = 0,
        SR_4000 = 1,
        SR_2000 = 3,
        SR_1000 = 7,
    };

    enum class AccelFS : uint8_t {
        G2 = 0,  // 2**14 LSB/g (resolution)
        G4 = 1,  // 2**13
        G8 = 2,  // 2**12
        G16 = 3, // 2**11
    };

    // degrees per second resolution
    enum class GyroFS : uint8_t {
        D250 = 0,  // 131 °/s
        D500 = 1,  // 65.5
        D1000 = 2, // 32.8
        D2000 = 3, // 16.4
    };

    struct Configuration {
        std::array<bool, 3> accel_self_tests;
        AccelFS accel_full_scale;
        std::array<bool, 3> gyro_self_tests;
        GyroFS gyro_full_scale;
    };

    I2C_HandleTypeDef &i2c_handle;
    Configuration configuration;

    bool dumb_init() const {
        auto const &self = *this;

        if (uint8_t whoami_result = self[MPU9250::Register::WHO_AM_I]; whoami_result != 0x71)
            return false;

        self[MPU9250::Register::PWR_MGMT_1] = 0x00;
        // self[MPU9250::Register::PWR_MGMT_2] = 0x00;

        self[MPU9250::Register::SMPLRT_DIV] = static_cast<uint8_t>(SampleRate::SR_1000);
        self[MPU9250::Register::GYRO_CONFIG] = (static_cast<uint8_t>(configuration.gyro_self_tests[0]) << 7)
            | (static_cast<uint8_t>(configuration.gyro_self_tests[1]) << 6)
            | (static_cast<uint8_t>(configuration.gyro_self_tests[2]) << 5)
            | (static_cast<uint8_t>(configuration.gyro_full_scale) << 3) | 3;
        self[MPU9250::Register::ACCEL_CONFIG_1] = (static_cast<uint8_t>(configuration.accel_self_tests[0]) << 7)
            | (static_cast<uint8_t>(configuration.accel_self_tests[1]) << 6)
            | (static_cast<uint8_t>(configuration.accel_self_tests[2]) << 5)
            | (static_cast<uint8_t>(configuration.accel_full_scale) << 3);
        self[MPU9250::Register::ACCEL_CONFIG_2] = 0x00;

        return true;
    }

    Stf::Vector<float, 3> get_accel() const {
        float scale = 1.f;
        switch (configuration.accel_full_scale) {
        case AccelFS::G2:
            scale = 1.f / 16384.f;
            break;
        case AccelFS::G4:
            scale = 1.f / 8192.f;
            break;
        case AccelFS::G8:
            scale = 1.f / 4096.f;
            break;
        case AccelFS::G16:
            scale = 1.f / 2048.f;
            break;
        }

        return get_mapped_vector(Register::ACCEL_XOUT_H, scale);
    }

    Stf::Vector<float, 3> get_gyro() const {
        float scale = 1.f;
        switch (configuration.gyro_full_scale) {
        case GyroFS::D250:
            scale = 1.f / 131.f;
            break;
        case GyroFS::D500:
            scale = 1.f / 62.5f;
            break;
        case GyroFS::D1000:
            scale = 1.f / 32.8f;
            break;
        case GyroFS::D2000:
            scale = 1.f / 16.4f;
            break;
        }

        return get_mapped_vector(Register::GYRO_XOUT_H, scale);
    }

private:
    constexpr I2CAddress operator[](Register reg) const {
        return {
            .i2c_handle = i2c_handle,
            .dev_addr = device_address_hi,
            .mem_addr = static_cast<uint16_t>(reg),
        };
    }

    Stf::Vector<int16_t, 3> get_raw_vector(Register base_address) const {
        std::array<uint8_t, 6> raw_data;
        (*this)[base_address].read(raw_data);

        const auto raw_ax = (static_cast<uint16_t>(raw_data[0]) << 8) | static_cast<uint16_t>(raw_data[1]);
        const auto raw_ay = (static_cast<uint16_t>(raw_data[2]) << 8) | static_cast<uint16_t>(raw_data[3]);
        const auto raw_az = (static_cast<uint16_t>(raw_data[4]) << 8) | static_cast<uint16_t>(raw_data[5]);

        return Stf::vector<int16_t>(
            static_cast<int16_t>(raw_ax), static_cast<int16_t>(raw_ay), static_cast<int16_t>(raw_az));
    }

    Stf::Vector<float, 3> get_mapped_vector(Register base_address, float scale) const {
        const auto raw_vec = get_raw_vector(base_address);
        const auto mapped = Stf::map(raw_vec, [scale](auto v) -> float { return static_cast<float>(v) * scale; });

        return Stf::vector(mapped);
    }
};

}
