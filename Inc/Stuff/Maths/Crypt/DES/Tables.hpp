#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace Stf::DES::Detail {

using PTableInitial = std::array<uint64_t, 64>;
using PTableFinal = std::array<uint64_t, 64>;
using PETableHB = std::array<uint64_t, 48>;
using STableSBox = std::array<std::array<uint64_t, 64>, 8>;
using PTableFFinal = std::array<uint64_t, 32>;
using PTablePC1 = std::array<uint64_t, 56>;
using PTablePC2 = std::array<uint64_t, 48>;

/*struct Tables {
    std::array<uint64_t, 64> p_initial; //64 to 64, plaintext/ciphertext
    std::array<uint64_t, 64> p_final; //64 to 64, ciphertext/plaintext
    std::array<uint64_t, 48> ep_plaintext_expansion; //32 to 48, half-block
    std::array<std::array<uint64_t, 64>, 8> s_boxes; //6 to 4, s-boxes
    std::array<uint64_t, 32> p_feistel_final; //32 to 32, half-block

    std::array<uint64_t, 56> cp_key_sched_pc_1; //64 to 56, key
    std::array<uint64_t, 48> cp_key_sched_pc_2; //56 to 48, half-keys
};

inline constexpr Tables k_default_tables {};*/

/// DES tables are MSB-1 which is very inconvenient for bit manipulation. this
/// function will transform an N-tuple of DES lookup values into a std::array of
/// `sizeof(T) * CHAR_BIT` elements containing a lookup adequate for
/// `Stf::permute_bits`\n
/// If N < std::numeric_limits<T>::digits then the table is considered a
/// compression table and the contents will be practically LSB-aligned
template<typename T, size_t N> constexpr auto permutation_table(const T (&lookup)[N], T source_bits = std::numeric_limits<T>::digits) {
    std::array<T, N> ret;
    for (auto i = 0uz; i < N; i++)
        ret[N - i - 1] = source_bits - lookup[i];
    return ret;
}

template<typename T, size_t N> constexpr std::array<T, N> substitution_table(const T (&lookup)[N]) {
    std::array<T, N> ret;
    for (auto i = 0uz; i < N; i++) {
        const auto column = static_cast<T>(i % 16);
        const auto row = i / 16;
        const auto first = (row & 0x2) != 0;
        const auto sixth = (row & 0x1) != 0;

        const auto actual_key = (static_cast<T>(column) << 1) | (first ? static_cast<T>(0x20) : 0) | (sixth ? static_cast<T>(0x1) : 0);

        ret[actual_key] = lookup[i];
    }
    return ret;
}

// 64 to 64, plaintext IP (P) box
inline constexpr auto k_initial_permutation_table = permutation_table<uint64_t>(
    {
        58, 50, 42, 34, 26, 18, 10, 2, //
        60, 52, 44, 36, 28, 20, 12, 4, //
        62, 54, 46, 38, 30, 22, 14, 6, //
        64, 56, 48, 40, 32, 24, 16, 8, //
        57, 49, 41, 33, 25, 17, 9, 1,  //
        59, 51, 43, 35, 27, 19, 11, 3, //
        61, 53, 45, 37, 29, 21, 13, 5, //
        63, 55, 47, 39, 31, 23, 15, 7, //
    },
    64);

// 64 to 64, ciphertext FP (P) box
inline constexpr auto k_final_permutation_table = permutation_table<uint64_t>(
    {
        40, 8, 48, 16, 56, 24, 64, 32, //
        39, 7, 47, 15, 55, 23, 63, 31, //
        38, 6, 46, 14, 54, 22, 62, 30, //
        37, 5, 45, 13, 53, 21, 61, 29, //
        36, 4, 44, 12, 52, 20, 60, 28, //
        35, 3, 43, 11, 51, 19, 59, 27, //
        34, 2, 42, 10, 50, 18, 58, 26, //
        33, 1, 41, 9, 49, 17, 57, 25,  //
    },
    64);

// 32 to 48, plaintext E box
inline constexpr auto k_expansion_table = permutation_table<uint64_t>(
    {
        32, 1, 2, 3, 4, 5, 4, 5,        //
        6, 7, 8, 9, 8, 9, 10, 11,       //
        12, 13, 12, 13, 14, 15, 16, 17, //
        16, 17, 18, 19, 20, 21, 20, 21, //
        22, 23, 24, 25, 24, 25, 26, 27, //
        28, 29, 28, 29, 30, 31, 32, 1,  //
    },
    32);

// 6 bits to 4 bits, S boxes
inline constexpr std::array<std::array<uint64_t, 64>, 8> k_sub_tables {
    substitution_table<uint64_t>({
        14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7, //
        0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8, //
        4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0, //
        15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13, //
    }),
    substitution_table<uint64_t>({
        15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10, //
        3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5, //
        0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15, //
        13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9, //
    }),
    substitution_table<uint64_t>({
        10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8, //
        13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1, //
        13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7, //
        1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12, //
    }),
    substitution_table<uint64_t>({
        7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15, //
        13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9, //
        10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4, //
        3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14, //
    }),
    substitution_table<uint64_t>({
        2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9, //
        14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6, //
        4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14, //
        11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3, //
    }),
    substitution_table<uint64_t>({
        12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11, //
        10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8, //
        9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6, //
        4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13, //
    }),
    substitution_table<uint64_t>({
        4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1, //
        13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6, //
        1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2, //
        6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12, //
    }),
    substitution_table<uint64_t>({
        13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7, //
        1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2, //
        7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8, //
        2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11, //
    }),
};

// 32 to 32, F function final permutation P box
inline constexpr auto k_f_final_p_table = permutation_table<uint64_t>(
    {
        16, 7, 20, 21, 29, 12, 28, 17, //
        1, 15, 23, 26, 5, 18, 31, 10,  //
        2, 8, 24, 14, 32, 27, 3, 9,    //
        19, 13, 30, 6, 22, 11, 4, 25,  //
    },
    32);

inline constexpr uint64_t k_key_shifts[16] = { 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };

// 64 to 56, permuted choice 1 (initial key transform)
inline constexpr auto k_key_sched_pc_1 = permutation_table<uint64_t>(
    {
        57, 49, 41, 33, 25, 17, 9, 1,   //
        58, 50, 42, 34, 26, 18, 10, 2,  //
        59, 51, 43, 35, 27, 19, 11, 3,  //
        60, 52, 44, 36, 63, 55, 47, 39, //
        31, 23, 15, 7, 62, 54, 46, 38,  //
        30, 22, 14, 6, 61, 53, 45, 37,  //
        29, 21, 13, 5, 28, 20, 12, 4,   //
    },
    64);

// 56 to 48, permuted choice 2 (round key transform)
inline constexpr auto k_key_sched_pc_2 = permutation_table<uint64_t>(
    {
        14, 17, 11, 24, 1, 5, 3, 28,    //
        15, 6, 21, 10, 23, 19, 12, 4,   //
        26, 8, 16, 7, 27, 20, 13, 2,    //
        41, 52, 31, 37, 47, 55, 30, 40, //
        51, 45, 33, 48, 44, 49, 39, 56, //
        34, 53, 46, 42, 50, 36, 29, 32, //
    },
    56);

}
