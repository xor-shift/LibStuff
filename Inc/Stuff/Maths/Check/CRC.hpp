#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>

#include <Stuff/Maths/Bit.hpp>

namespace Stf::Concepts {

template<typename T>
concept CRCDescription = requires() {
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

#include "./CRCDescriptions.ipp"

namespace Stf {

namespace Detail {

template<Concepts::CRCDescription Desc, bool UseLookup> struct CRCRemainderGenerator;

template<Concepts::CRCDescription Desc> struct CRCRemainderGenerator<Desc, false> {
    using sum_type = typename Desc::sum_type;
    using desc_type = Desc;
    static constexpr sum_type mask
        = (sizeof(sum_type) * std::numeric_limits<uint8_t>::digits == desc_type::width) ? ~0 : ((1 << desc_type::width) - 1);

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

template<Concepts::CRCDescription Desc> struct CRCRemainderGenerator<Desc, true> {
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

}

template<Concepts::CRCDescription Desc, bool UseLookup = false> struct CRCState {
    using sum_type = typename Desc::sum_type;
    using desc_type = Desc;
    using generator_type = Detail::CRCRemainderGenerator<Desc, UseLookup>;
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
