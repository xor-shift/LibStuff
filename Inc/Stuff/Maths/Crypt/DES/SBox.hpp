// Credit to Matthew Kwan for these S-Boxes
// http://fgrieu.free.fr/Mattew%20Kwan%20-%20Reducing%20the%20Gate%20Count%20of%20Bitslice%20DES.pdf
// http://darkside.com.au/bitslice/

#pragma once

#include <cstdint>

namespace Stf::DES::Detail::Bitslice::X86 {

// x([0-9]{1,2})
// x[$1-1]

#define SBOX_ARGS uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6, uint64_t &out1, uint64_t &out2, uint64_t &out3, uint64_t &out4

template<size_t N> constexpr void mk_sbox(SBOX_ARGS);

template<> constexpr void mk_sbox<1>(SBOX_ARGS) {
    const auto x1 = ~a4;
    const auto x2 = ~a1;
    const auto x3 = a4 ^ a3;
    const auto x4 = x3 ^ x2;
    const auto x5 = a3 | x2;
    const auto x6 = x5 & x1;
    const auto x7 = a6 | x6;
    const auto x8 = x4 ^ x7;
    const auto x9 = x1 | x2;
    const auto x10 = a6 & x9;
    const auto x11 = x7 ^ x10;
    const auto x12 = a2 | x11;
    const auto x13 = x8 ^ x12;
    const auto x14 = x9 ^ x13;
    const auto x15 = a6 | x14;
    const auto x16 = x1 ^ x15;
    const auto x17 = ~x14;
    const auto x18 = x17 & x3;
    const auto x19 = a2 | x18;
    const auto x20 = x16 ^ x19;
    const auto x21 = a5 | x20;
    const auto x22 = x13 ^ x21;
    const auto x23 = a3 | x4;
    const auto x24 = ~x23;
    const auto x25 = a6 | x24;
    const auto x26 = x6 ^ x25;
    const auto x27 = x1 & x8;
    const auto x28 = a2 | x27;
    const auto x29 = x26 ^ x28;
    const auto x30 = x1 | x8;
    const auto x31 = x30 ^ x6;
    const auto x32 = x5 & x14;
    const auto x33 = x32 ^ x8;
    const auto x34 = a2 & x33;
    const auto x35 = x31 ^ x34;
    const auto x36 = a5 | x35;
    const auto x37 = x29 ^ x36;
    const auto x38 = a3 & x10;
    const auto x39 = x38 | x4;
    const auto x40 = a3 & x33;
    const auto x41 = x40 ^ x25;
    const auto x42 = a2 | x41;
    const auto x43 = x39 ^ x42;
    const auto x44 = a3 | x26;
    const auto x45 = x44 ^ x14;
    const auto x46 = a1 | x8;
    const auto x47 = x46 ^ x20;
    const auto x48 = a2 | x47;
    const auto x49 = x45 ^ x48;
    const auto x50 = a5 & x49;
    const auto x51 = x43 ^ x50;
    const auto x52 = x8 ^ x40;
    const auto x53 = a3 ^ x11;
    const auto x54 = x53 & x5;
    const auto x55 = a2 | x54;
    const auto x56 = x52 ^ x55;
    const auto x57 = a6 | x4;
    const auto x58 = x57 ^ x38;
    const auto x59 = x13 & x56;
    const auto x60 = a2 & x59;
    const auto x61 = x58 ^ x60;
    const auto x62 = a5 & x61;
    const auto x63 = x56 ^ x62;

    out1 ^= x37;
    out2 ^= x51;
    out3 ^= x63;
    out4 ^= x22;
}

template<> constexpr void mk_sbox<2>(SBOX_ARGS) {
    const auto x1 = ~a5;
    const auto x2 = ~a1;
    const auto x3 = a5 ^ a6;
    const auto x4 = x3 ^ x2;
    const auto x5 = x4 ^ a2;
    const auto x6 = a6 | x1;
    const auto x7 = x6 | x2;
    const auto x8 = a2 & x7;
    const auto x9 = a6 ^ x8;
    const auto x10 = a3 & x9;
    const auto x11 = x5 ^ x10;
    const auto x12 = a2 & x9;
    const auto x13 = a5 ^ x6;
    const auto x14 = a3 | x13;
    const auto x15 = x12 ^ x14;
    const auto x16 = a4 & x15;
    const auto x17 = x11 ^ x16;
    const auto x18 = a5 | a1;
    const auto x19 = a6 | x18;
    const auto x20 = x13 ^ x19;
    const auto x21 = x20 ^ a2;
    const auto x22 = a6 | x4;
    const auto x23 = x22 & x17;
    const auto x24 = a3 | x23;
    const auto x25 = x21 ^ x24;
    const auto x26 = a6 | x2;
    const auto x27 = a5 & x2;
    const auto x28 = a2 | x27;
    const auto x29 = x26 ^ x28;
    const auto x30 = x3 ^ x27;
    const auto x31 = x2 ^ x19;
    const auto x32 = a2 & x31;
    const auto x33 = x30 ^ x32;
    const auto x34 = a3 & x33;
    const auto x35 = x29 ^ x34;
    const auto x36 = a4 | x35;
    const auto x37 = x25 ^ x36;
    const auto x38 = x21 & x32;
    const auto x39 = x38 ^ x5;
    const auto x40 = a1 | x15;
    const auto x41 = x40 ^ x13;
    const auto x42 = a3 | x41;
    const auto x43 = x39 ^ x42;
    const auto x44 = x28 | x41;
    const auto x45 = a4 & x44;
    const auto x46 = x43 ^ x45;
    const auto x47 = x19 & x21;
    const auto x48 = x47 ^ x26;
    const auto x49 = a2 & x33;
    const auto x50 = x49 ^ x21;
    const auto x51 = a3 & x50;
    const auto x52 = x48 ^ x51;
    const auto x53 = x18 & x28;
    const auto x54 = x53 & x50;
    const auto x55 = a4 | x54;
    const auto x56 = x52 ^ x55;

    out1 ^= x46;
    out2 ^= x17;
    out3 ^= x37;
    out4 ^= x56;
}

template<> constexpr void mk_sbox<3>(SBOX_ARGS) {
    const auto x1 = ~a5;
    const auto x2 = ~a6;
    const auto x3 = a5 & a3;
    const auto x4 = x3 ^ a6;
    const auto x5 = a4 & x1;
    const auto x6 = x4 ^ x5;
    const auto x7 = x6 ^ a2;
    const auto x8 = a3 & x1;
    const auto x9 = a5 ^ x2;
    const auto x10 = a4 | x9;
    const auto x11 = x8 ^ x10;
    const auto x12 = x7 & x11;
    const auto x13 = a5 ^ x11;
    const auto x14 = x13 | x7;
    const auto x15 = a4 & x14;
    const auto x16 = x12 ^ x15;
    const auto x17 = a2 & x16;
    const auto x18 = x11 ^ x17;
    const auto x19 = a1 & x18;
    const auto x20 = x7 ^ x19;
    const auto x21 = a3 ^ a4;
    const auto x22 = x21 ^ x9;
    const auto x23 = x2 | x4;
    const auto x24 = x23 ^ x8;
    const auto x25 = a2 | x24;
    const auto x26 = x22 ^ x25;
    const auto x27 = a6 ^ x23;
    const auto x28 = x27 | a4;
    const auto x29 = a3 ^ x15;
    const auto x30 = x29 | x5;
    const auto x31 = a2 | x30;
    const auto x32 = x28 ^ x31;
    const auto x33 = a1 | x32;
    const auto x34 = x26 ^ x33;
    const auto x35 = a3 ^ x9;
    const auto x36 = x35 | x5;
    const auto x37 = x4 | x29;
    const auto x38 = x37 ^ a4;
    const auto x39 = a2 | x38;
    const auto x40 = x36 ^ x39;
    const auto x41 = a6 & x11;
    const auto x42 = x41 | x6;
    const auto x43 = x34 ^ x38;
    const auto x44 = x43 ^ x41;
    const auto x45 = a2 & x44;
    const auto x46 = x42 ^ x45;
    const auto x47 = a1 | x46;
    const auto x48 = x40 ^ x47;
    const auto x49 = x2 | x38;
    const auto x50 = x49 ^ x13;
    const auto x51 = x27 ^ x28;
    const auto x52 = a2 | x51;
    const auto x53 = x50 ^ x52;
    const auto x54 = x12 & x23;
    const auto x55 = x54 & x52;
    const auto x56 = a1 | x55;
    const auto x57 = x53 ^ x56;

    out1 ^= x34;
    out2 ^= x57;
    out3 ^= x48;
    out4 ^= x20;
}

template<> constexpr void mk_sbox<4>(SBOX_ARGS) {
    const auto x1 = ~a1;
    const auto x2 = ~a3;
    const auto x3 = a1 | a3;
    const auto x4 = a5 & x3;
    const auto x5 = x1 ^ x4;
    const auto x6 = a2 | a3;
    const auto x7 = x5 ^ x6;
    const auto x8 = a1 & a5;
    const auto x9 = x8 ^ x3;
    const auto x10 = a2 & x9;
    const auto x11 = a5 ^ x10;
    const auto x12 = a4 & x11;
    const auto x13 = x7 ^ x12;
    const auto x14 = x2 ^ x4;
    const auto x15 = a2 & x14;
    const auto x16 = x9 ^ x15;
    const auto x17 = x5 & x14;
    const auto x18 = a5 ^ x2;
    const auto x19 = a2 | x18;
    const auto x20 = x17 ^ x19;
    const auto x21 = a4 | x20;
    const auto x22 = x16 ^ x21;
    const auto x23 = a6 & x22;
    const auto x24 = x13 ^ x23;
    const auto x25 = ~x13;
    const auto x26 = a6 | x22;
    const auto x27 = x25 ^ x26;
    const auto x28 = a2 & x11;
    const auto x29 = x28 ^ x17;
    const auto x30 = a3 ^ x10;
    const auto x31 = x30 ^ x19;
    const auto x32 = a4 & x31;
    const auto x33 = x29 ^ x32;
    const auto x34 = x25 ^ x33;
    const auto x35 = a2 & x34;
    const auto x36 = x24 ^ x35;
    const auto x37 = a4 | x34;
    const auto x38 = x36 ^ x37;
    const auto x39 = a6 & x38;
    const auto x40 = x33 ^ x39;
    const auto x41 = x26 ^ x38;
    const auto x42 = x41 ^ x40;

    out1 ^= x27;
    out2 ^= x24;
    out3 ^= x42;
    out4 ^= x40;
}

template<> constexpr void mk_sbox<5>(SBOX_ARGS) {
    const auto x1 = ~a6;
    const auto x2 = ~a3;
    const auto x3 = x1 | x2;
    const auto x4 = x3 ^ a4;
    const auto x5 = a1 & x3;
    const auto x6 = x4 ^ x5;
    const auto x7 = a6 | a4;
    const auto x8 = x7 ^ a3;
    const auto x9 = a3 | x7;
    const auto x10 = a1 | x9;
    const auto x11 = x8 ^ x10;
    const auto x12 = a5 & x11;
    const auto x13 = x6 ^ x12;
    const auto x14 = ~x4;
    const auto x15 = x14 & a6;
    const auto x16 = a1 | x15;
    const auto x17 = x8 ^ x16;
    const auto x18 = a5 | x17;
    const auto x19 = x10 ^ x18;
    const auto x20 = a2 | x19;
    const auto x21 = x13 ^ x20;
    const auto x22 = x2 | x15;
    const auto x23 = x22 ^ a6;
    const auto x24 = a4 ^ x22;
    const auto x25 = a1 & x24;
    const auto x26 = x23 ^ x25;
    const auto x27 = a1 ^ x11;
    const auto x28 = x27 & x22;
    const auto x29 = a5 | x28;
    const auto x30 = x26 ^ x29;
    const auto x31 = a4 | x27;
    const auto x32 = ~x31;
    const auto x33 = a2 | x32;
    const auto x34 = x30 ^ x33;
    const auto x35 = x2 ^ x15;
    const auto x36 = a1 & x35;
    const auto x37 = x14 ^ x36;
    const auto x38 = x5 ^ x7;
    const auto x39 = x38 & x34;
    const auto x40 = a5 | x39;
    const auto x41 = x37 ^ x40;
    const auto x42 = x2 ^ x5;
    const auto x43 = x42 & x16;
    const auto x44 = x4 & x27;
    const auto x45 = a5 & x44;
    const auto x46 = x43 ^ x45;
    const auto x47 = a2 | x46;
    const auto x48 = x41 ^ x47;
    const auto x49 = x24 & x48;
    const auto x50 = x49 ^ x5;
    const auto x51 = x11 ^ x30;
    const auto x52 = x51 | x50;
    const auto x53 = a5 & x52;
    const auto x54 = x50 ^ x53;
    const auto x55 = x14 ^ x19;
    const auto x56 = x55 ^ x34;
    const auto x57 = x4 ^ x16;
    const auto x58 = x57 & x30;
    const auto x59 = a5 & x58;
    const auto x60 = x56 ^ x59;
    const auto x61 = a2 | x60;
    const auto x62 = x54 ^ x61;

    out1 ^= x48;
    out2 ^= x34;
    out3 ^= x21;
    out4 ^= x62;
}

template<> constexpr void mk_sbox<6>(SBOX_ARGS) {
    const auto x1 = ~a2;
    const auto x2 = ~a5;
    const auto x3 = a2 ^ a6;
    const auto x4 = x3 ^ x2;
    const auto x5 = x4 ^ a1;
    const auto x6 = a5 & a6;
    const auto x7 = x6 | x1;
    const auto x8 = a5 & x5;
    const auto x9 = a1 & x8;
    const auto x10 = x7 ^ x9;
    const auto x11 = a4 & x10;
    const auto x12 = x5 ^ x11;
    const auto x13 = a6 ^ x10;
    const auto x14 = x13 & a1;
    const auto x15 = a2 & a6;
    const auto x16 = x15 ^ a5;
    const auto x17 = a1 & x16;
    const auto x18 = x2 ^ x17;
    const auto x19 = a4 | x18;
    const auto x20 = x14 ^ x19;
    const auto x21 = a3 & x20;
    const auto x22 = x12 ^ x21;
    const auto x23 = a6 ^ x18;
    const auto x24 = a1 & x23;
    const auto x25 = a5 ^ x24;
    const auto x26 = a2 ^ x17;
    const auto x27 = x26 | x6;
    const auto x28 = a4 & x27;
    const auto x29 = x25 ^ x28;
    const auto x30 = ~x26;
    const auto x31 = a6 | x29;
    const auto x32 = ~x31;
    const auto x33 = a4 & x32;
    const auto x34 = x30 ^ x33;
    const auto x35 = a3 & x34;
    const auto x36 = x29 ^ x35;
    const auto x37 = x6 ^ x34;
    const auto x38 = a5 & x23;
    const auto x39 = x38 ^ x5;
    const auto x40 = a4 | x39;
    const auto x41 = x37 ^ x40;
    const auto x42 = x16 | x24;
    const auto x43 = x42 ^ x1;
    const auto x44 = x15 ^ x24;
    const auto x45 = x44 ^ x31;
    const auto x46 = a4 | x45;
    const auto x47 = x43 ^ x46;
    const auto x48 = a3 | x47;
    const auto x49 = x41 ^ x48;
    const auto x50 = x5 | x38;
    const auto x51 = x50 ^ x6;
    const auto x52 = x8 & x31;
    const auto x53 = a4 | x52;
    const auto x54 = x51 ^ x53;
    const auto x55 = x30 & x43;
    const auto x56 = a3 | x55;
    const auto x57 = x54 ^ x56;

    out1 ^= x49;
    out2 ^= x22;
    out3 ^= x57;
    out4 ^= x36;
}

template<> constexpr void mk_sbox<7>(SBOX_ARGS) {
    const auto x1 = ~a2;
    const auto x2 = ~a5;
    const auto x3 = a2 & a4;
    const auto x4 = x3 ^ a5;
    const auto x5 = x4 ^ a3;
    const auto x6 = a4 & x4;
    const auto x7 = x6 ^ a2;
    const auto x8 = a3 & x7;
    const auto x9 = a1 ^ x8;
    const auto x10 = a6 | x9;
    const auto x11 = x5 ^ x10;
    const auto x12 = a4 & x2;
    const auto x13 = x12 | a2;
    const auto x14 = a2 | x2;
    const auto x15 = a3 & x14;
    const auto x16 = x13 ^ x15;
    const auto x17 = x6 ^ x11;
    const auto x18 = a6 | x17;
    const auto x19 = x16 ^ x18;
    const auto x20 = a1 & x19;
    const auto x21 = x11 ^ x20;
    const auto x22 = a2 | x21;
    const auto x23 = x22 ^ x6;
    const auto x24 = x23 ^ x15;
    const auto x25 = x5 ^ x6;
    const auto x26 = x25 | x12;
    const auto x27 = a6 | x26;
    const auto x28 = x24 ^ x27;
    const auto x29 = x1 & x19;
    const auto x30 = x23 & x26;
    const auto x31 = a6 & x30;
    const auto x32 = x29 ^ x31;
    const auto x33 = a1 | x32;
    const auto x34 = x28 ^ x33;
    const auto x35 = a4 & x16;
    const auto x36 = x35 | x1;
    const auto x37 = a6 & x36;
    const auto x38 = x11 ^ x37;
    const auto x39 = a4 & x13;
    const auto x40 = a3 | x7;
    const auto x41 = x39 ^ x40;
    const auto x42 = x1 | x24;
    const auto x43 = a6 | x42;
    const auto x44 = x41 ^ x43;
    const auto x45 = a1 | x44;
    const auto x46 = x38 ^ x45;
    const auto x47 = x8 ^ x44;
    const auto x48 = x6 ^ x15;
    const auto x49 = a6 | x48;
    const auto x50 = x47 ^ x49;
    const auto x51 = x19 ^ x44;
    const auto x52 = a4 ^ x25;
    const auto x53 = x52 & x46;
    const auto x54 = a6 & x53;
    const auto x55 = x51 ^ x54;
    const auto x56 = a1 | x55;
    const auto x57 = x50 ^ x56;

    out1 ^= x21;
    out2 ^= x46;
    out3 ^= x57;
    out4 ^= x34;
}

template<> constexpr void mk_sbox<8>(SBOX_ARGS) {
    const auto x1 = ~a1;
    const auto x2 = ~a4;
    const auto x3 = a3 ^ x1;
    const auto x4 = a3 | x1;
    const auto x5 = x4 ^ x2;
    const auto x6 = a5 | x5;
    const auto x7 = x3 ^ x6;
    const auto x8 = x1 | x5;
    const auto x9 = x2 ^ x8;
    const auto x10 = a5 & x9;
    const auto x11 = x8 ^ x10;
    const auto x12 = a2 & x11;
    const auto x13 = x7 ^ x12;
    const auto x14 = x6 ^ x9;
    const auto x15 = x3 & x9;
    const auto x16 = a5 & x8;
    const auto x17 = x15 ^ x16;
    const auto x18 = a2 | x17;
    const auto x19 = x14 ^ x18;
    const auto x20 = a6 | x19;
    const auto x21 = x13 ^ x20;
    const auto x22 = a5 | x3;
    const auto x23 = x22 & x2;
    const auto x24 = ~a3;
    const auto x25 = x24 & x8;
    const auto x26 = a5 & x4;
    const auto x27 = x25 ^ x26;
    const auto x28 = a2 | x27;
    const auto x29 = x23 ^ x28;
    const auto x30 = a6 & x29;
    const auto x31 = x13 ^ x30;
    const auto x32 = x5 ^ x6;
    const auto x33 = x32 ^ x22;
    const auto x34 = a4 | x13;
    const auto x35 = a2 & x34;
    const auto x36 = x33 ^ x35;
    const auto x37 = a1 & x33;
    const auto x38 = x37 ^ x8;
    const auto x39 = a1 ^ x23;
    const auto x40 = x39 & x7;
    const auto x41 = a2 & x40;
    const auto x42 = x38 ^ x41;
    const auto x43 = a6 | x42;
    const auto x44 = x36 ^ x43;
    const auto x45 = a1 ^ x10;
    const auto x46 = x45 ^ x22;
    const auto x47 = ~x7;
    const auto x48 = x47 & x8;
    const auto x49 = a2 | x48;
    const auto x50 = x46 ^ x49;
    const auto x51 = x19 ^ x29;
    const auto x52 = x51 | x38;
    const auto x53 = a6 & x52;
    const auto x54 = x50 ^ x53;

    out1 ^= x21;
    out2 ^= x54;
    out3 ^= x44;
    out4 ^= x31;
}

template<size_t N = 8> void helper(uint64_t (&input)[48], uint64_t (&output)[32]) {
    const auto in_start_idx = (N - 1) * 6;
    const auto out_start_idx = (N - 1) * 4;
    mk_sbox<N>(                    //
        input[in_start_idx + 0],   //
        input[in_start_idx + 1],   //
        input[in_start_idx + 2],   //
        input[in_start_idx + 3],   //
        input[in_start_idx + 4],   //
        input[in_start_idx + 5],   //
        output[out_start_idx + 0], //
        output[out_start_idx + 1], //
        output[out_start_idx + 2], //
        output[out_start_idx + 3]  //
    );
    if constexpr (N != 1)
        return helper<N - 1>(input, output);
}

}
