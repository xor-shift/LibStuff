#pragma once

#include <Stuff/Maths/Bit.hpp>
#include <Stuff/Maths/Crypt/DES/SBox.hpp>
#include <Stuff/Maths/Crypt/DES/Tables.hpp>

namespace Stf::DES::Detail {

// 64 to 56, discards parity
constexpr uint64_t remove_key_parity(uint64_t key) {
    uint64_t ret = 0;

    if consteval {
        const auto arr = std::bit_cast<std::array<uint8_t, 8>>(key);

        for (auto i = 0uz; i < 8uz; i++) {
            auto index = std::endian::native == std::endian::big ? i : 7 - i;
            const auto b = arr[index];

            ret <<= 7;
            ret |= b & 0x7F;
        }
    } else {
        auto* arr = reinterpret_cast<uint8_t*>(&key);

        for (auto i = 0uz; i < 8uz; i++) {
            auto index = std::endian::native == std::endian::big ? i : 7 - i;
            const auto b = arr[index];

            ret <<= 7;
            ret |= b & 0x7F;
        }
    }

    return ret;
}

// 64 to 56, strips parity and applies PC1
constexpr uint64_t prepare_key(uint64_t key, PTablePC1 const& pc_1 = k_key_sched_pc_1) {
    // const auto stripped = remove_key_parity(key);
    return Stf::permute_bits(key, pc_1);
}

template<typename T> constexpr T rotl(T v, T bits, T n) {
    const auto one = static_cast<T>(1);

    const auto shifted = (v << n) & ((one << bits) - one);
    const auto carried = v >> static_cast<T>(bits - n);

    return shifted | carried;
}

template<typename T, size_t N, bool MSB0 = true> constexpr void rotl(std::span<T, N> arr, size_t n) {
    for (auto i = 0uz; i < N; i++) {
        const auto j = (i + n) % N;
        using std::swap;
        swap(arr[i], arr[j]);
    }
}

// 56 to 56 and 48, turns subkey_(n-1) to subkey_n_
constexpr std::pair<uint64_t, uint64_t> subkey_n(uint64_t key, uint64_t round, PTablePC2 const& pc_2 = k_key_sched_pc_2) {
    const auto left_key = (key >> 28) & 0xFFFFFFF;
    const auto right_key = key & 0xFFFFFFF;
    const auto shift_by = k_key_shifts[round];

    const auto shifted_left_key = rotl<uint64_t>(left_key, 28, shift_by);
    const auto shifted_right_key = rotl<uint64_t>(right_key, 28, shift_by);

    const auto new_key = ((shifted_left_key << 28) | shifted_right_key);
    const auto permuted_key = Stf::permute_bits(new_key, pc_2);

    /*fmt::print("left    : {:07X} -> {:07X}\n", left_key, shifted_left_key);
    fmt::print("right   : {:07X} -> {:07X}\n", right_key, shifted_right_key);
    fmt::print("combined: {:014X}\n", new_key);
    fmt::print("permuted: {:012X}\n", permuted_key);*/

    return { new_key, permuted_key };
}

// 48 to 32, S-box transforms of the Feistel block. uses k_sub_tables
constexpr uint64_t sbox_transform_lookup(uint64_t data, STableSBox const& s_boxes = k_sub_tables) {
    uint64_t ret = 0;
    for (uint64_t i = 0ull; i < 8; i++) {
        const auto s_index = (data >> (48 - (i + 1) * 6)) & 0x3F;
        const auto s_res = s_boxes[i][s_index];
        ret <<= 4;
        ret |= s_res;
    }

    return ret;
}

// 32 to 32
constexpr uint64_t feistel_block(uint64_t /* 32 bits */ data, uint64_t round_key, //
    PETableHB const& expansion_table = k_expansion_table, PTableFFinal const& final_table = k_f_final_p_table) {
    const auto expanded_data = Stf::permute_bits(data, expansion_table);
    const auto xored_data = expanded_data ^ round_key;
    const auto s_result = sbox_transform_lookup(xored_data);
    return Stf::permute_bits(s_result, final_table);
}

constexpr std::array<uint64_t, 16> key_schedule(uint64_t key) {
    std::array<uint64_t, 16> round_keys;
    uint64_t prev_key = key;

    for (uint64_t i = 0; i < 16; i++) {
        const auto [new_key, round_key] = Detail::subkey_n(prev_key, i);
        round_keys[i] = round_key;
        prev_key = new_key;
    }

    return round_keys;
}

}
