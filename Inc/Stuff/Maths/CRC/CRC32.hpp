#pragma once

#include <array>
#include <concepts>

#include "../Bit.hpp"

namespace Stf {

namespace Detail {

enum class CRC32Polys : uint32_t {
    Base = 0x04C11DB7UL,         // a lot of shiz including ethernet, PNG, Gzip, Bzip2, POSIX's cksum
    Castagnoli = 0x1EDC6F41,     // iSCSI, SCTP, G.hn. Btrfs, ext4, Ceph (can be accelerated using SSE4.2)
    Koopman_1_3_28 = 0x741B8CD7, // good for ethernet frame lengths
    Koopman_1_1_30 = 0x32583499, // good for ethernet frame lengths
};

template<uint32_t Poly> constexpr uint32_t crc32_lookup_element(uint8_t index) {
    // auto ret = static_cast<uint32_t>(index);
    auto ret = static_cast<uint32_t>(index) << 24;
    for (uint8_t z = 0; z < 8; z++) {
        // ret = (ret & 1) == 1 ? (ret >> 1) ^ Stf::reverse_bits(Poly) : ret >> 1;
        ret = (ret & 0x80000000) != 0 ? (ret << 1) ^ Poly : ret << 1;
    }
    return ret;
}

template<uint32_t Poly> constexpr std::array<uint32_t, 256> crc32_create_lookup() {
    std::array<uint32_t, 256> temp { 0 };

    for (size_t index = 0; index < (1 << 8); index++) {
        temp[index] = crc32_lookup_element<Poly>(index);
    }

    return temp;
}

template<uint32_t Poly> constexpr uint32_t crc32_advance(uint32_t state, uint8_t byte) {
    // const auto index = static_cast<uint8_t>(state & 0xFF) ^ byte;
    // return crc32_lookup_element<Poly>(index) ^ (state >> 8);
    const auto index = static_cast<uint8_t>(state >> 24) ^ byte;
    return crc32_lookup_element<Poly>(index) ^ (state << 8);
}

constexpr uint32_t crc32_advance(uint32_t state, uint8_t byte, std::array<uint32_t, 256> const& lookup) {
    // const auto index = static_cast<uint8_t>(state & 0xFF) ^ byte;
    // return lookup[index] ^ (state >> 8);
    const auto index = static_cast<uint8_t>(state >> 24) ^ byte;
    return lookup[index] ^ (state << 8);
}

}

template<uint32_t Poly = static_cast<uint32_t>(Detail::CRC32Polys::Base)> struct CRC32SWBackend {
    using value_type = uint32_t;

    uint32_t state = ~0u;

    constexpr void update(uint8_t b) { state = Detail::crc32_advance<Poly>(state, b); }

    constexpr value_type finish() { return state = ~state; }
};

template<typename Backend = CRC32SWBackend<>> struct CRCContext {
    using value_type = typename Backend::value_type;

    Backend backend = {};

    constexpr void update(uint8_t b) { backend.update(b); }

    constexpr void update(char const* str, size_t len) {
        while (len-- != 0)
            update(*str++);
    }

    constexpr void update(char const* str) {
        for (char c = *str; c; c = *++str)
            update(c);
    }

    constexpr value_type finish() { return backend.finish(); }
};

}

namespace Stf::Detail::CRC {

namespace Concepts {

template<typename T>
concept Description = requires() {
                          { T::name } -> std::convertible_to<const char*>;

                          typename T::sum_type;

                          { T::width } -> std::convertible_to<typename T::sum_type>;
                          { T::poly } -> std::convertible_to<typename T::sum_type>;
                          { T::init } -> std::convertible_to<typename T::sum_type>;
                          { T::reflect } -> std::convertible_to<bool>;
                          { T::xor_out } -> std::convertible_to<typename T::sum_type>;

                          { T::check } -> std::convertible_to<typename T::sum_type>;
                      };

}

namespace Descriptions {


/* https://reveng.sourceforge.io/crc-catalogue/ dev console script for scraping data:
console.log(( () => {
    let res = "";

    let blocks = document.querySelectorAll("p code");
    for (let block of blocks) {
        let heading_elem = block.parentElement.previousElementSibling.querySelectorAll("[name^='crc.cat.']")[0];
        res += heading_elem.innerHTML + ' ';
        res += block.innerHTML + '\n';
    }

return res;
} )())
 */
//regex: ^CRC-([0-9]+)\/([^ ]+) width=([0-9]+)  poly=(0x[0-9a-f]+)  init=(0x[0-9a-f]+)  refin=(true|false)  refout=(true|false)  xorout=(0x[0-9a-f]+)  check=(0x[0-9a-f]+)  residue=[^ ]+  name=("[^ ]+")
//replace: MAKE_CRC_DESCRIPTION(CRC$1$2, $10, void, $3, $4ul, $5ul, $6, $8ul, $9ul)
//replace - with _ afterwards

#define MAKE_CRC_DESCRIPTION(STRUCTNAME, NAME, SUMTYPE, WIDTH, POLY, INIT, REFLECT, XOROUT, CHECK) \
    struct STRUCTNAME {                                                                            \
        static constexpr const char* name = NAME;                                                  \
                                                                                                   \
                                                                                                   \
        using sum_type = SUMTYPE;                                                                  \
        static constexpr sum_type width = WIDTH;                                                   \
        static constexpr sum_type poly = POLY;                                                     \
        static constexpr sum_type init = INIT;                                                     \
        static constexpr bool reflect = REFLECT;                                                   \
        static constexpr sum_type xor_out = XOROUT;                                                \
                                                                                                   \
        static constexpr sum_type check = CHECK;                                                   \
    };

MAKE_CRC_DESCRIPTION(
    CRC32ISOHDLC, "CRC-32/ISO-HDLC", uint32_t, 32, 0x04c11db7ul, 0xfffffffful, true, 0xfffffffful, 0xcbf43926ul)
MAKE_CRC_DESCRIPTION(
    CRC32BZIP2, "CRC-32/BZIP2", uint32_t, 32, 0x04c11db7ul, 0xfffffffful, false, 0xfffffffful, 0xfc891918ul)
MAKE_CRC_DESCRIPTION(CRC16CCITT, "CRC-16/KERMIT", uint16_t, 16, 0x1021, 0, true, 0, 0x2189)

MAKE_CRC_DESCRIPTION(CRC3GSM, "CRC-3/GSM", uint8_t, 3, 0x3ul, 0x0ul, false, 0x7ul, 0x4ul)
MAKE_CRC_DESCRIPTION(CRC3ROHC, "CRC-3/ROHC", uint8_t, 3, 0x3ul, 0x7ul, true, 0x0ul, 0x6ul)
MAKE_CRC_DESCRIPTION(CRC4G704, "CRC-4/G-704", uint8_t, 4, 0x3ul, 0x0ul, true, 0x0ul, 0x7ul)
MAKE_CRC_DESCRIPTION(CRC4Interlaken, "CRC-4/INTERLAKEN", uint8_t, 4, 0x3ul, 0xful, false, 0xful, 0xbul)
MAKE_CRC_DESCRIPTION(CRC5EPCC1G2, "CRC-5/EPC-C1G2", uint8_t, 5, 0x09ul, 0x09ul, false, 0x00ul, 0x00ul)
MAKE_CRC_DESCRIPTION(CRC5G704, "CRC-5/G-704", uint8_t, 5, 0x15ul, 0x00ul, true, 0x00ul, 0x07ul)
MAKE_CRC_DESCRIPTION(CRC5USB, "CRC-5/USB", uint8_t, 5, 0x05ul, 0x1ful, true, 0x1ful, 0x19ul)
MAKE_CRC_DESCRIPTION(CRC6CDMA2000A, "CRC-6/CDMA2000-A", uint8_t, 6, 0x27ul, 0x3ful, false, 0x00ul, 0x0dul)
MAKE_CRC_DESCRIPTION(CRC6CDMA2000B, "CRC-6/CDMA2000-B", uint8_t, 6, 0x07ul, 0x3ful, false, 0x00ul, 0x3bul)
MAKE_CRC_DESCRIPTION(CRC6DARC, "CRC-6/DARC", uint8_t, 6, 0x19ul, 0x00ul, true, 0x00ul, 0x26ul)
MAKE_CRC_DESCRIPTION(CRC6G704, "CRC-6/G-704", uint8_t, 6, 0x03ul, 0x00ul, true, 0x00ul, 0x06ul)
MAKE_CRC_DESCRIPTION(CRC6GSM, "CRC-6/GSM", uint8_t, 6, 0x2ful, 0x00ul, false, 0x3ful, 0x13ul)
MAKE_CRC_DESCRIPTION(CRC7MMC, "CRC-7/MMC", uint8_t, 7, 0x09ul, 0x00ul, false, 0x00ul, 0x75ul)
MAKE_CRC_DESCRIPTION(CRC7ROHC, "CRC-7/ROHC", uint8_t, 7, 0x4ful, 0x7ful, true, 0x00ul, 0x53ul)
MAKE_CRC_DESCRIPTION(CRC7UMTS, "CRC-7/UMTS", uint8_t, 7, 0x45ul, 0x00ul, false, 0x00ul, 0x61ul)
MAKE_CRC_DESCRIPTION(CRC8AUTOSAR, "CRC-8/AUTOSAR", uint8_t, 8, 0x2ful, 0xfful, false, 0xfful, 0xdful)
MAKE_CRC_DESCRIPTION(CRC8Bluetooth, "CRC-8/BLUETOOTH", uint8_t, 8, 0xa7ul, 0x00ul, true, 0x00ul, 0x26ul)
MAKE_CRC_DESCRIPTION(CRC8CDMA2000, "CRC-8/CDMA2000", uint8_t, 8, 0x9bul, 0xfful, false, 0x00ul, 0xdaul)
MAKE_CRC_DESCRIPTION(CRC8DARC, "CRC-8/DARC", uint8_t, 8, 0x39ul, 0x00ul, true, 0x00ul, 0x15ul)
MAKE_CRC_DESCRIPTION(CRC8DVBS2, "CRC-8/DVB-S2", uint8_t, 8, 0xd5ul, 0x00ul, false, 0x00ul, 0xbcul)
MAKE_CRC_DESCRIPTION(CRC8GSMA, "CRC-8/GSM-A", uint8_t, 8, 0x1dul, 0x00ul, false, 0x00ul, 0x37ul)
MAKE_CRC_DESCRIPTION(CRC8GSMB, "CRC-8/GSM-B", uint8_t, 8, 0x49ul, 0x00ul, false, 0xfful, 0x94ul)
MAKE_CRC_DESCRIPTION(CRC8HITAG, "CRC-8/HITAG", uint8_t, 8, 0x1dul, 0xfful, false, 0x00ul, 0xb4ul)
MAKE_CRC_DESCRIPTION(CRC8I4321, "CRC-8/I-432-1", uint8_t, 8, 0x07ul, 0x00ul, false, 0x55ul, 0xa1ul)
MAKE_CRC_DESCRIPTION(CRC8ICODE, "CRC-8/I-CODE", uint8_t, 8, 0x1dul, 0xfdul, false, 0x00ul, 0x7eul)
MAKE_CRC_DESCRIPTION(CRC8LTE, "CRC-8/LTE", uint8_t, 8, 0x9bul, 0x00ul, false, 0x00ul, 0xeaul)
MAKE_CRC_DESCRIPTION(CRC8MaximDow, "CRC-8/MAXIM-DOW", uint8_t, 8, 0x31ul, 0x00ul, true, 0x00ul, 0xa1ul)
MAKE_CRC_DESCRIPTION(CRC8MIFAREMAD, "CRC-8/MIFARE-MAD", uint8_t, 8, 0x1dul, 0xc7ul, false, 0x00ul, 0x99ul)
MAKE_CRC_DESCRIPTION(CRC8NRSC5, "CRC-8/NRSC-5", uint8_t, 8, 0x31ul, 0xfful, false, 0x00ul, 0xf7ul)
MAKE_CRC_DESCRIPTION(CRC8Opensafety, "CRC-8/OPENSAFETY", uint8_t, 8, 0x2ful, 0x00ul, false, 0x00ul, 0x3eul)
MAKE_CRC_DESCRIPTION(CRC8ROHC, "CRC-8/ROHC", uint8_t, 8, 0x07ul, 0xfful, true, 0x00ul, 0xd0ul)
MAKE_CRC_DESCRIPTION(CRC8SAEJ1850, "CRC-8/SAE-J1850", uint8_t, 8, 0x1dul, 0xfful, false, 0xfful, 0x4bul)
MAKE_CRC_DESCRIPTION(CRC8SMBUS, "CRC-8/SMBUS", uint8_t, 8, 0x07ul, 0x00ul, false, 0x00ul, 0xf4ul)
MAKE_CRC_DESCRIPTION(CRC8TECH3250, "CRC-8/TECH-3250", uint8_t, 8, 0x1dul, 0xfful, true, 0x00ul, 0x97ul)
MAKE_CRC_DESCRIPTION(CRC8WCDMA, "CRC-8/WCDMA", uint8_t, 8, 0x9bul, 0x00ul, true, 0x00ul, 0x25ul)
MAKE_CRC_DESCRIPTION(CRC10ATM, "CRC-10/ATM", uint16_t, 10, 0x233ul, 0x000ul, false, 0x000ul, 0x199ul)
MAKE_CRC_DESCRIPTION(CRC10CDMA2000, "CRC-10/CDMA2000", uint16_t, 10, 0x3d9ul, 0x3fful, false, 0x000ul, 0x233ul)
MAKE_CRC_DESCRIPTION(CRC10GSM, "CRC-10/GSM", uint16_t, 10, 0x175ul, 0x000ul, false, 0x3fful, 0x12aul)
MAKE_CRC_DESCRIPTION(CRC11FlexRay, "CRC-11/FLEXRAY", uint16_t, 11, 0x385ul, 0x01aul, false, 0x000ul, 0x5a3ul)
MAKE_CRC_DESCRIPTION(CRC11UMTS, "CRC-11/UMTS", uint16_t, 11, 0x307ul, 0x000ul, false, 0x000ul, 0x061ul)
MAKE_CRC_DESCRIPTION(CRC12CDMA2000, "CRC-12/CDMA2000", uint16_t, 12, 0xf13ul, 0xffful, false, 0x000ul, 0xd4dul)
MAKE_CRC_DESCRIPTION(CRC12DECT, "CRC-12/DECT", uint16_t, 12, 0x80ful, 0x000ul, false, 0x000ul, 0xf5bul)
MAKE_CRC_DESCRIPTION(CRC12GSM, "CRC-12/GSM", uint16_t, 12, 0xd31ul, 0x000ul, false, 0xffful, 0xb34ul)
//MAKE_CRC_DESCRIPTION(CRC12UMTS, "CRC-12/UMTS", uint16_t, 12, 0x80ful, 0x000ul, false, 0x000ul, 0xdaful)
MAKE_CRC_DESCRIPTION(CRC13BBC, "CRC-13/BBC", uint16_t, 13, 0x1cf5ul, 0x0000ul, false, 0x0000ul, 0x04faul)
//MAKE_CRC_DESCRIPTION(CRC14DARC, "CRC-14/DARC", uint16_t, 14, 0x0805ul, 0x0000ul, true, 0x0000ul, 0x082dul)
MAKE_CRC_DESCRIPTION(CRC14GSM, "CRC-14/GSM", uint16_t, 14, 0x202dul, 0x0000ul, false, 0x3ffful, 0x30aeul)
MAKE_CRC_DESCRIPTION(CRC15CAN, "CRC-15/CAN", uint16_t, 15, 0x4599ul, 0x0000ul, false, 0x0000ul, 0x059eul)
MAKE_CRC_DESCRIPTION(CRC15MPT1327, "CRC-15/MPT1327", uint16_t, 15, 0x6815ul, 0x0000ul, false, 0x0001ul, 0x2566ul)

#undef MAKE_CRC_DESCRIPTION

}

template<Concepts::Description Desc, bool UseLookup> struct CRCRemainderGenerator;

template<Concepts::Description Desc> struct CRCRemainderGenerator<Desc, false> {
    using sum_type = typename Desc::sum_type;
    using desc_type = Desc;
    static constexpr sum_type mask
        = (sizeof(sum_type) * CHAR_BIT == desc_type::width) ? ~0 : ((1 << desc_type::width) - 1);

    constexpr sum_type operator[](uint8_t index) const {
        auto ret = static_cast<uint32_t>(index);

        if constexpr (!desc_type::reflect && desc_type::width >= 8)
            ret <<= desc_type::width - 8;

        constexpr sum_type xor_value = desc_type::reflect ? Stf::reverse_bits(desc_type::poly) : desc_type::poly;
        constexpr sum_type check_bit = desc_type::reflect ? 1 : 1 << (desc_type::width - 1);

        for (uint8_t z = 0; z < 8; z++) {
            const bool do_shift = (ret & check_bit) != 0;

            if constexpr (desc_type::reflect)
                ret >>= 1;
            else
                ret <<= 1;

            if (do_shift)
                ret ^= xor_value;
        }

        return ret & mask;
    }
};

template<Concepts::Description Desc> struct CRCRemainderGenerator<Desc, true> {
    using sum_type = typename Desc::sum_type;
    using desc_type = Desc;
    using base_type = CRCRemainderGenerator<desc_type, false>;
    static constexpr sum_type mask = base_type::mask;

    static constexpr base_type base {};

    static constexpr std::array<sum_type, 256> lookup = ([] {
        std::array<sum_type, 256> ret;

        for (size_t index = 0; index < 256; index++)
            ret[index] = base[index];

        return ret;
    })();

    constexpr sum_type operator[](uint8_t index) const { return lookup[index]; }
};

template<Concepts::Description Desc, bool UseLookup = false> struct CRCState {
    using sum_type = typename Desc::sum_type;
    using desc_type = Desc;
    using generator_type = CRCRemainderGenerator<Desc, UseLookup>;
    static constexpr bool uses_lookup = UseLookup;
    static constexpr sum_type mask = generator_type::mask;

    static constexpr generator_type remainder_generator {};

    sum_type state /*: desc_type::width*/ = desc_type::init;

    using value_type = uint8_t;
    static constexpr size_t value_size = 8;

    constexpr std::enable_if_t<value_size <= desc_type::width, void> update(uint8_t b) {
        auto index = static_cast<uint8_t>((state >> (!desc_type::reflect ? desc_type::width - 8 : 0)) & mask) ^ b;

        if constexpr (desc_type::reflect)
            state = remainder_generator[index] ^ (state >> 8);
        else
            state = remainder_generator[index] ^ (state << 8);

        state &= mask;
    }
    constexpr sum_type finished_value() const { return state ^ desc_type::xor_out; }
};

}

namespace Stf::CRC {

namespace Concepts {

using namespace Detail::CRC::Concepts;

}

namespace Descriptions = Detail::CRC::Descriptions;
using Detail::CRC::CRCState;

}
