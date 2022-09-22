#pragma once

#include <ranges>

#include <Stuff/Maths/Crypt/DES/Funcs.hpp>

namespace Stf::DES {

namespace Detail {

template<bool Reverse> constexpr uint64_t routine(uint64_t data, uint64_t key, PETableHB const& expansion_table = k_expansion_table) {
    const auto input_permuted = Stf::permute_bits(data, k_initial_permutation_table);
    const auto round_keys = Detail::key_schedule(key);

    auto left = input_permuted >> 32;
    auto right = input_permuted & 0xFFFF'FFFFul;

    for (uint64_t i = 0; i < 16; i++) {
        const auto round_key = round_keys[Reverse ? 15 - i : i];

        const auto f_right = feistel_block(right, round_key, expansion_table);
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
    return Stf::permute_bits(pre_permutation, k_final_permutation_table);
}

constexpr std::array<uint64_t, 48> get_crypt_expansion_block(std::string_view salt) {
    auto ret = k_expansion_table;

    for (auto i = 0uz; i < 2uz; i++) {
        auto c = salt[i];

        if (c > 'Z')
            c -= 6;
        if (c > '9')
            c -= 7;

        c -= '.';

        for (auto j = 0uz; j < 6uz; j++) {
            if (((c >> j) % 2) != 0) {
                const auto idx_left = 47 - (6uz * i + j);
                const auto idx_right = 47 - (6uz * i + j + 24);
                using std::swap;
                swap(ret[idx_left], ret[idx_right]);
            }
        }
    }

    return ret;
}

constexpr std::string crypt_base64(std::string_view salt, uint64_t data) {
    bool bits[66] {};

    std::string ret { salt };
    ret.reserve(66);

    for (uint64_t i = 0; i < 64; i++) {
        bits[i] = ((data >> (63 - i)) & 1) != 0;
    }

    for (auto i = 0uz; i < 11uz; i++) {
        char c = 0;
        for (auto j = 0uz; j < 6uz; j++) {
            c <<= 1;
            c |= bits[6 * i + j];
        }

        c += '.';

        if (c > '9')
            c += 7;

        if (c > 'Z')
            c += 6;

        ret += c;
    }

    return ret;
}

constexpr uint64_t get_crypt_key(std::string_view pw) {
    uint64_t ret = 0;

    for (auto i = 0uz; i < std::min(pw.size(), 8uz); i++) {
        ret <<= 8;
        ret |= pw[i] & 0x7F;
    }
    ret <<= (65 - pw.size() * 8);

    return ret;
}

template<size_t TripSize = 8, size_t SaltSuffixSize = 2, auto SaltSuffix = std::array<char, SaltSuffixSize> { 'H', '.' }>
constexpr std::string_view get_tripkey(char (&output)[TripSize + SaltSuffixSize], std::string_view raw_key) {
    size_t size = 0;

    auto push_str = [&](std::string_view s) {
        const auto to_copy = std::min(s.size(), 8 - size);
        std::copy_n(s.begin(), to_copy, output + size);
        size += to_copy;
    };

    auto push_char = [&](char c) {
        if (size == TripSize)
            return;

        std::pair<char, std::string_view> replacements[] {
            { '&', "&amp;" },
            { '>', "&gt;" },
            { '<', "&lt" },
            { '"', "&quot;" },
        };

        auto it = std::ranges::find_if(replacements, [c](auto v) { return v.first == c; });
        if (it == replacements + 4) {
            output[size++] = c;
        } else {
            push_str(it->second);
        }
    };

    for (char c : raw_key) {
        push_char(c);
        if (size == TripSize)
            break;
    }

    auto salt_transform = [](char c) -> char {
        if (c >= ':' && c <= '@')
            return 'A' + (c - ':');

        if (c >= '[' && c <= '`')
            return 'a' + (c - '[');

        return c;
    };

    std::ranges::copy(SaltSuffix, output + size);

    for (auto i = 0uz; i < SaltSuffixSize; i++) {
        output[size + i] = salt_transform(output[i + 1]);
    }

    return { output, size };
}

}

constexpr uint64_t encrypt(uint64_t plaintext, uint64_t raw_key) {
    const auto key = Detail::prepare_key(raw_key);
    return Detail::routine<false>(plaintext, key);
}

constexpr uint64_t decrypt(uint64_t ciphertext, uint64_t raw_key) {
    const auto key = Detail::prepare_key(raw_key);
    return Detail::routine<true>(ciphertext, key);
}

constexpr void encrypt_bitslice(std::span<uint64_t, 64> plaintext, std::span<uint64_t, 64> key) { }

// UNIX v7 crypt(3)
constexpr std::string crypt(std::string_view pw, std::string_view salt) {
    if (salt.size() < 2)
        return "";

    if (salt.size() != 2)
        salt = salt.substr(0, 2);

    if (pw.size() > 8)
        pw = pw.substr(0, 8);

    const auto expansion_table = Detail::get_crypt_expansion_block(salt);
    const auto raw_key = Detail::get_crypt_key(pw);
    const auto key = Detail::prepare_key(raw_key);

    uint64_t block = 0;

    for (auto i = 0uz; i < 25; i++)
        block = Detail::routine<false>(block, key, expansion_table);

    return Detail::crypt_base64(salt, block);
}

constexpr std::string tripcode(std::string_view raw_tripkey) {
    char tripkey_data[10];
    const auto tripkey = Detail::get_tripkey(tripkey_data, raw_tripkey);

    if (tripkey.empty())
        return "";

    auto trip = crypt(tripkey, { tripkey_data + tripkey.size(), 2 });

    return trip.substr(3);
}

}
