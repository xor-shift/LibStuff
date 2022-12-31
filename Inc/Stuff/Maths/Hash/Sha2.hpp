#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>

#include <Stuff/Maths/Bit.hpp>

namespace Stf::Hash {

constexpr std::string format_digest(auto const& digest, bool capital = false) {
    using T = std::decay_t<decltype(digest[0])>;

    std::array<char, sizeof(T) * 2> buffer;
    std::string ret;
    ret.reserve(digest.size() * sizeof(T) * 2);

    for (auto v : digest) {
        auto res = std::to_chars(buffer.data(), buffer.data() + buffer.size(), v, 16);
        auto it = std::copy_backward(buffer.data(), res.ptr, end(buffer));
        std::fill(begin(buffer), it, '0');

        if (capital) {
            for (auto& c : buffer) {
                // why are locale related functions not constexpr?
                // c = std::toupper(c);

                if ('z' < c || c < 'a')
                    continue;

                c -= 'a' - 'A';
            }
        }

        copy(begin(buffer), end(buffer), back_inserter(ret));
    }

    return ret;
}

}

namespace Stf::Hash::SHA2 {

template<typename T>
concept SHA2Properties = requires() //
{
    { T::name } -> std::convertible_to<std::string_view>;
    { T::chunk_bits } -> std::convertible_to<size_t>;

    typename T::length_type;
    typename T::state_type;
    typename T::digest_type;

    { T::default_h } -> std::convertible_to<typename T::state_type>;
    T::round_values;

    T::sum_0(T::default_h[0]);
    T::sum_1(T::default_h[0]);
    T::sig_0(T::default_h[0]);
    T::sig_1(T::default_h[0]);
};

struct SHA256Properties {
    static constexpr const char* name = "SHA-2/256";
    static constexpr size_t chunk_bits = 512;

    using length_type = uint64_t;
    using state_type = std::array<uint32_t, 8>;
    using digest_type = std::array<uint32_t, 8>;
    using schedule_type = std::array<uint32_t, 64>;

    static constexpr state_type default_h {
        0x6a09e667U, //
        0xbb67ae85U, //
        0x3c6ef372U, //
        0xa54ff53aU, //
        0x510e527fU, //
        0x9b05688cU, //
        0x1f83d9abU, //
        0x5be0cd19U, //
    };

    static constexpr schedule_type round_values {
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, //
        0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U, //
        0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, 0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, //
        0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U, //
        0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U, //
        0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U, //
        0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U, //
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, 0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U, //
    };

    static constexpr uint32_t sum_0(uint32_t x) { return std::rotr(x, 2) ^ std::rotr(x, 13) ^ std::rotr(x, 22); }

    static constexpr uint32_t sum_1(uint32_t x) { return std::rotr(x, 6) ^ std::rotr(x, 11) ^ std::rotr(x, 25); }

    static constexpr uint32_t sig_0(uint32_t x) { return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3); }

    static constexpr uint32_t sig_1(uint32_t x) { return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10); }
};

struct SHA224Properties : SHA256Properties {
    static constexpr const char* name = "SHA-2/224";

    using digest_type = std::array<uint32_t, 7>;

    static constexpr state_type default_h = {
        0xc1059ed8U, //
        0x367cd507U, //
        0x3070dd17U, //
        0xf70e5939U, //
        0xffc00b31U, //
        0x68581511U, //
        0x64f98fa7U, //
        0xbefa4fa4U, //
    };
};

struct SHA512Properties {
    static constexpr const char* name = "SHA-2/512";
    static constexpr size_t chunk_bits = 1024;

    using length_type = __uint128_t;
    using state_type = std::array<uint64_t, 8>;
    using digest_type = std::array<uint64_t, 8>;
    using schedule_type = std::array<uint64_t, 80>;

    static constexpr state_type default_h {
        0x6a09e667f3bcc908UL, 0xbb67ae8584caa73bUL, 0x3c6ef372fe94f82bUL, 0xa54ff53a5f1d36f1UL, //
        0x510e527fade682d1UL, 0x9b05688c2b3e6c1fUL, 0x1f83d9abfb41bd6bUL, 0x5be0cd19137e2179UL, //
    };

    static constexpr schedule_type round_values {
        0x428a2f98d728ae22UL, 0x7137449123ef65cdUL, 0xb5c0fbcfec4d3b2fUL, 0xe9b5dba58189dbbcUL, 0x3956c25bf348b538UL, //
        0x59f111f1b605d019UL, 0x923f82a4af194f9bUL, 0xab1c5ed5da6d8118UL, 0xd807aa98a3030242UL, 0x12835b0145706fbeUL, //
        0x243185be4ee4b28cUL, 0x550c7dc3d5ffb4e2UL, 0x72be5d74f27b896fUL, 0x80deb1fe3b1696b1UL, 0x9bdc06a725c71235UL, //
        0xc19bf174cf692694UL, 0xe49b69c19ef14ad2UL, 0xefbe4786384f25e3UL, 0x0fc19dc68b8cd5b5UL, 0x240ca1cc77ac9c65UL, //
        0x2de92c6f592b0275UL, 0x4a7484aa6ea6e483UL, 0x5cb0a9dcbd41fbd4UL, 0x76f988da831153b5UL, 0x983e5152ee66dfabUL, //
        0xa831c66d2db43210UL, 0xb00327c898fb213fUL, 0xbf597fc7beef0ee4UL, 0xc6e00bf33da88fc2UL, 0xd5a79147930aa725UL, //
        0x06ca6351e003826fUL, 0x142929670a0e6e70UL, 0x27b70a8546d22ffcUL, 0x2e1b21385c26c926UL, 0x4d2c6dfc5ac42aedUL, //
        0x53380d139d95b3dfUL, 0x650a73548baf63deUL, 0x766a0abb3c77b2a8UL, 0x81c2c92e47edaee6UL, 0x92722c851482353bUL, //
        0xa2bfe8a14cf10364UL, 0xa81a664bbc423001UL, 0xc24b8b70d0f89791UL, 0xc76c51a30654be30UL, 0xd192e819d6ef5218UL, //
        0xd69906245565a910UL, 0xf40e35855771202aUL, 0x106aa07032bbd1b8UL, 0x19a4c116b8d2d0c8UL, 0x1e376c085141ab53UL, //
        0x2748774cdf8eeb99UL, 0x34b0bcb5e19b48a8UL, 0x391c0cb3c5c95a63UL, 0x4ed8aa4ae3418acbUL, 0x5b9cca4f7763e373UL, //
        0x682e6ff3d6b2b8a3UL, 0x748f82ee5defb2fcUL, 0x78a5636f43172f60UL, 0x84c87814a1f0ab72UL, 0x8cc702081a6439ecUL, //
        0x90befffa23631e28UL, 0xa4506cebde82bde9UL, 0xbef9a3f7b2c67915UL, 0xc67178f2e372532bUL, 0xca273eceea26619cUL, //
        0xd186b8c721c0c207UL, 0xeada7dd6cde0eb1eUL, 0xf57d4f7fee6ed178UL, 0x06f067aa72176fbaUL, 0x0a637dc5a2c898a6UL, //
        0x113f9804bef90daeUL, 0x1b710b35131c471bUL, 0x28db77f523047d84UL, 0x32caab7b40c72493UL, 0x3c9ebe0a15c9bebcUL, //
        0x431d67c49c100d4cUL, 0x4cc5d4becb3e42b6UL, 0x597f299cfc657e2aUL, 0x5fcb6fab3ad6faecUL, 0x6c44198c4a475817UL, //
    };

    static constexpr uint64_t sum_0(uint64_t x) { return std::rotr(x, 28) ^ std::rotr(x, 34) ^ std::rotr(x, 39); }

    static constexpr uint64_t sum_1(uint64_t x) { return std::rotr(x, 14) ^ std::rotr(x, 18) ^ std::rotr(x, 41); }

    static constexpr uint64_t sig_0(uint64_t x) { return std::rotr(x, 1) ^ std::rotr(x, 8) ^ (x >> 7); }

    static constexpr uint64_t sig_1(uint64_t x) { return std::rotr(x, 19) ^ std::rotr(x, 61) ^ (x >> 6); }
};

struct SHA384Properties : SHA512Properties {
    static constexpr const char* name = "SHA-2/384";

    using digest_type = std::array<uint64_t, 6>;

    static constexpr state_type default_h {
        0xcbbb9d5dc1059ed8UL, 0x629a292a367cd507UL, 0x9159015a3070dd17UL, 0x152fecd8f70e5939UL, //
        0x67332667ffc00b31UL, 0x8eb44a8768581511UL, 0xdb0c2e0d64f98fa7UL, 0x47b5481dbefa4fa4UL, //
    };
};

namespace Detail {

static constexpr SHA512Properties::state_type generate_mods512_iv() {
    auto iv = SHA512Properties::default_h;
    for (auto& v : iv)
        v ^= 0xa5a5a5a5a5a5a5a5UL;
    return iv;
}

}

struct ModSHA512Properties : SHA512Properties {
    static constexpr state_type default_h = Detail::generate_mods512_iv();
};

template<size_t NWords>
    requires(NWords <= 8) && (NWords != 0) && (NWords != 6)
struct Sha512TProperties : SHA512Properties { };

template<SHA2Properties Props> struct SHA2State {
    constexpr void reset() {
        m_bit_size = 0;
        m_pending_data = 0;
        std::copy(begin(Props::default_h), end(Props::default_h), begin(m_digest));
    }

    constexpr void update(uint8_t byte) {
        m_data[m_pending_data++] = byte;

        if (m_pending_data >= m_data.size())
            finish_block();
    }

    template<typename T, size_t Extent> constexpr void update(std::span<T, Extent> data) {
        if constexpr (std::is_same_v<std::remove_const<T>, uint8_t>) {
            update_bulk(data);
        } else {
            for (auto b : data)
                update(b);
        }
    }

    template<typename Char, typename Traits = std::char_traits<Char>>
    constexpr void update(std::basic_string_view<Char, Traits> data) {
        return update(std::span(data));
    }

    template<typename T>
        requires std::integral<T>
    constexpr void update(T v) {
        if consteval {
            auto arr = std::bit_cast<std::array<uint8_t, sizeof(T)>>(v);
            if (std::endian::native != std::endian::big)
                std::reverse(begin(arr), end(arr));

            update(std::span(arr));
        } else {
            if (std::endian::native != std::endian::big)
                v = Stf::reverse_bytes(v);

            auto* p = reinterpret_cast<uint8_t*>(&v);

            update(std::span { p, sizeof(T) });
        }
    }

    constexpr void update(char c) { return update(std::bit_cast<uint8_t>(c)); }
    constexpr void update(const char* c) { return update(std::string_view(c)); }

    constexpr typename Props::digest_type finish() {
        typename Props::length_type cached_bit_length = m_bit_size + m_pending_data * 8;

        update('\x80');
        while (m_pending_data != m_data.size() - sizeof(cached_bit_length))
            update('\0');

        update(cached_bit_length);

        if constexpr (std::is_same_v<typename Props::state_type, typename Props::digest_type>)
            return m_digest;

        typename Props::digest_type ret;
        std::copy_n(begin(m_digest), ret.size(), begin(ret));
        return ret;
    }

private:
    size_t m_bit_size = 0;
    size_t m_pending_data = 0;
    std::array<uint8_t, Props::chunk_bits / 8> m_data;

    typename Props::state_type m_digest { Props::default_h };
    typename Props::schedule_type m_schedule;

    constexpr void update_bulk(std::span<const uint8_t> data) {
        const auto data_sz = std::ranges::distance(data);
        const auto available_space = m_data.size() - m_pending_data;

        const auto cur_sz = std::min<size_t>(available_space, data_sz);
        const auto excess_sz = data_sz - cur_sz;

        // const auto cur_data = data | std::ranges::views::take(cur_sz);
        const auto cur_data = data.subspan(0, cur_sz);

        std::ranges::copy(cur_data, m_data.data() + m_pending_data);
        m_pending_data += cur_sz;
        if (m_pending_data == m_data.size()) {
            finish_block();
        }

        if (excess_sz == 0)
            return;

        // return update(data | std::ranges::views::drop(cur_sz));
        return update(data.subspan(cur_sz));
    }

    constexpr void finish_block() {
        data_to_schedule();
        transform();
        m_bit_size += Props::chunk_bits;
        m_pending_data = 0;
    }

    constexpr void data_to_schedule() {
        using target_type = typename Props::schedule_type::value_type;

        for (size_t i = 0; i < m_data.size() / sizeof(target_type); i++) {
            std::array<uint8_t, sizeof(target_type)> temp;

            std::copy(begin(m_data) + i * temp.size(), begin(m_data) + (i + 1) * sizeof(target_type), begin(temp));

            if constexpr (std::endian::native != std::endian::big)
                std::reverse(begin(temp), end(temp));

            m_schedule[i] = std::bit_cast<target_type>(temp);
        }
    }

    constexpr void transform() {
        for (size_t i = 16; i < m_schedule.size(); i++) {
            const auto s0 = Props::sig_0(m_schedule[i - 15]);
            const auto s1 = Props::sig_1(m_schedule[i - 2]);
            m_schedule[i] = s0 + s1 + m_schedule[i - 7] + m_schedule[i - 16];
        }

        typename Props::state_type args;
        std::copy(begin(m_digest), end(m_digest), begin(args));

        for (size_t i = 0; i < m_schedule.size(); i++) {
            auto& [a, b, c, d, e, f, g, h] = args;

            const auto s1 = Props::sum_1(e);
            const auto ch = (e & f) ^ (~e & g);
            const auto temp1 = h + s1 + ch + Props::round_values[i] + m_schedule[i];

            const auto s0 = Props::sum_0(a);
            const auto maj = (a & b) ^ (a & c) ^ (b & c);
            const auto temp2 = s0 + maj;

            /*std::rotate(begin(args), begin(args) + 1, end(args));
            e += temp1;
            a = temp1 + temp2;*/

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        for (size_t i = 0; i < 8; i++) {
            m_digest[i] += args[i];
        }
    }
};

}

namespace Stf::Hash {

using SHA224State = SHA2::SHA2State<SHA2::SHA224Properties>;
using SHA256State = SHA2::SHA2State<SHA2::SHA256Properties>;
using SHA384State = SHA2::SHA2State<SHA2::SHA384Properties>;
using SHA512State = SHA2::SHA2State<SHA2::SHA512Properties>;

}
