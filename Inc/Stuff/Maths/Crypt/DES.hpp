#pragma once

#include <Stuff/Maths/Crypt/DES/Funcs.hpp>

namespace Stf::Crypt::DES {

namespace Detail {

template<bool Reverse>
constexpr uint64_t routine(uint64_t data, uint64_t raw_key) {
    const auto input_permuted = Stf::permute_bits(data, Stf::Crypt::DES::Detail::k_initial_permutation_table);
    const auto round_keys = Detail::key_schedule(raw_key);

    auto left = input_permuted >> 32;
    auto right = input_permuted & 0xFFFF'FFFFul;

    for (uint64_t i = 0; i < 16; i++) {
        const auto round_key = round_keys[Reverse ? 15 - i : i];

        const auto f_right = Detail::feistel_block(right, round_key);
        left ^= f_right;

        if (i != 15)
            std::swap(left, right);

        /*fmt::print("\n");
        fmt::print("round  : {:02}\n", i + 1);
        fmt::print("subkey : {:012X}\n", round_key);
        fmt::print("feistel: {:08X}\n", f_right);
        fmt::print("data   : {:08X} {:08X}\n", left, right);*/
    }

    const auto pre_permutation = (left << 32) | right;
    return Stf::permute_bits(pre_permutation, Detail::k_final_permutation_table);
}

}

constexpr uint64_t crypt(uint64_t plaintext, uint64_t raw_key) {
    return Detail::routine<false>(plaintext, raw_key);
}

constexpr uint64_t decrypt(uint64_t ciphertext, uint64_t raw_key) {
    return Detail::routine<true>(ciphertext, raw_key);
}

}
