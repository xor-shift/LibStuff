#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

extern "C" void soft_uart_tx_tick_handler();

namespace Stf::UART {

struct PortConfig {
    inline static constexpr size_t max_start_bits = 1;
    inline static constexpr size_t max_stop_bits = 2;

    inline static constexpr size_t max_bits = 8 + PortConfig::max_start_bits + PortConfig::max_stop_bits;

    bool use_parity = true;
    bool invert_parity = false;
    uint16_t start_bits = 1;
    uint16_t stop_bits = 1;
    uint16_t word_size = 8;
};

inline static constexpr size_t n_ports = 6;
inline static constexpr size_t tx_buffer_size = 8;

void configure(size_t port, PortConfig config);

bool tx_bytes(size_t port, std::span<const uint8_t> data);

template<typename T>
    requires requires(T v) { typename std::enable_if_t<sizeof(T) == sizeof(uint8_t)>; }
inline bool tx_bytes(size_t port, const auto* data, size_t len) {
    return tx_bytes(port, { reinterpret_cast<const uint8_t*>(data), len });
}

extern void tx_callback(uint_fast32_t port, bool bit);

extern void tx_start_callback(size_t port);

extern void tx_complete_callback(size_t port);

}
