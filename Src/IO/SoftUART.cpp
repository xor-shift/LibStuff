#include <Stuff/IO/SoftUART.hpp>
#include <Stuff/Maths/Scalar.hpp>

#include <atomic>
#include <thread>

namespace Stf::UART {

static std::array<PortConfig, n_ports> port_configs {};

struct PortState {
    std::atomic_bool tx_in_progress = false;
    std::atomic_bool tx_lock = false;

    std::array<uint8_t, tx_buffer_size> tx_buffer {};
    uint_fast32_t tx_remaining = 0;
    uint_fast16_t tx_bit_buffer = 0b111111'1'0100'0001'0;
    uint_fast8_t tx_bits_remaining = 0;
};

static std::array<PortState, n_ports> port_states;

void configure(size_t port, PortConfig config) {
    tx_callback(port, true);
    port_configs[port] = config;
}

void setup_tx(size_t port, std::span<const uint8_t> data) {
    auto& state = port_states[port];

    std::copy(data.begin(), data.end(), state.tx_buffer.begin() + (8 - data.size()));
    state.tx_remaining = data.size();

    state.tx_in_progress = true;
}

bool tx_bytes(size_t port, std::span<const uint8_t> data) {
    auto& state = port_states[port];
    // if (state.tx_lock.exchange(true))
    //     return false;

    const size_t needed_txs = data.size() / tx_buffer_size + ((data.size() % tx_buffer_size == 0) ? 0 : 1);

    tx_start_callback(port);

    while (!data.empty()) {
        const auto consume = static_cast<ptrdiff_t>(std::min(tx_buffer_size, data.size()));

        std::span<const uint8_t> tx_span { data.begin(), data.begin() + consume };
        setup_tx(port, tx_span);

        while (state.tx_in_progress)
            std::this_thread::yield();

        data = { data.begin() + consume, data.end() };
    }

    tx_complete_callback(port);

    // state.tx_lock = false;

    return true;
}

void handle_single_port_tick_tx(uint_fast32_t port) {
    auto& state = port_states[port];
    auto const& config = port_configs[port];

    if (!state.tx_in_progress)
        return;

    if (state.tx_bits_remaining != 0) {
        const auto bit = state.tx_bit_buffer & 1;
        state.tx_bit_buffer >>= 1;
        --state.tx_bits_remaining;

        tx_callback(port, bit != 0);

        return;
    }

    if (state.tx_remaining == 0) {
        state.tx_in_progress = false;
        tx_callback(port, true);
        return;
    }

    const auto next_byte = state.tx_buffer[tx_buffer_size - state.tx_remaining--];
    state.tx_bit_buffer = 0xFFFF;

    if (config.use_parity) {
        state.tx_bit_buffer <<= 1;
        const uint16_t parity = (std::popcount(next_byte) + (config.invert_parity ? 1 : 0)) % 2;
        state.tx_bit_buffer |= parity;
    }

    state.tx_bit_buffer <<= config.word_size;
    state.tx_bit_buffer |= static_cast<uint16_t>(next_byte);

    state.tx_bit_buffer <<= config.start_bits;
    state.tx_bits_remaining = config.start_bits + config.word_size + (config.use_parity ? 1 : 0) + config.stop_bits;
}

[[gnu::weak]] extern void tx_callback(uint_fast32_t port, bool bit) { }

[[gnu::weak]] extern void tx_start_callback(size_t port) { }

[[gnu::weak]] extern void tx_complete_callback(size_t port) { }

}

extern "C" void soft_uart_tx_tick_handler() { Stf::UART::handle_single_port_tick_tx(0); };
