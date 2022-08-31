#pragma once

#include <algorithm>
#include <array>
#include <iterator>

namespace Stf::Gfx {

/// format examples:
/// RG11B10f -> 11 bits red and green, 10 bits blue, float representation\n
/// R5G6B5s -> 5 bits red and blue, 6 bits green, signed normalised representation\n
/// RGBA8 -> 8 bits red, green, and blue, unsigned normalised representation\n
/// representations:\n
/// ' ': unsigned normalised representation (float in range [0, 1])\n
/// 's': signed normalised representation (float in range [-1, 1])\n
/// 'f': regular IEEE754 float representation\n
/// 'i': signed integer representation\n
/// 'u': unsigned integer representation\n
/// for unsigned and signed normalised representations, the representation type
/// will be a float and any values out of range will be considered invalid.
/// use the norm_clamp member function on Gfx::Image to clamp all float values.
/// the function will be a no-op for f, i, and u representations.
enum class ColorFormat {
    /// 8 bits per channel RGB, unsigned integer repr
    RGB8u,
    /// 8 bits per channel RGBA, unsigned integer repr
    RGBA8u,
    /// 8 bits per channel BGRA, unsigned integer repr
    BGRA8u,
    /// 16 bits per channel RGB, unsigned integer repr
    RGB16u,
    /// 16 bits per channel RGBA, unsigned integer repr
    RGBA16u,
    /// 32 bits per channel RGB, unsigned integer repr
    RGB32u,
    /// 32 bits per channel RGBA, unsigned integer repr
    RGBA32u,
    /// 32 bits per channel RGB, floating point repr
    RGB32f,
    /// 32 bits per channel RGBA, floating point repr
    RGBA32f,
};

enum class ColorSpace {
    /// RGB in the sRGB color space, linear alpha
    SRGB,
    /// Linear RGB, linear alpha
    Linear,
};

template<ColorFormat, ColorSpace> struct ColorTraits;

template<ColorSpace CSpace, size_t ChannelCount, typename Repr> struct GenericColorTraits {
    static constexpr size_t channel_count = ChannelCount;
    using channel_type = Repr;
    static constexpr std::array<size_t, channel_count> bit_widths = ([] {
        std::array<size_t, channel_count> ret;
        std::fill(ret.begin(), ret.end(), sizeof(channel_type) * std::numeric_limits<unsigned char>::digits);
    })();
    using color_type = std::array<channel_type, channel_count>;

    template<size_t I> static constexpr channel_type get_channel(color_type c) {
        if constexpr (I < channel_count)
            return c[I];
        else
            return channel_type {};
    }

    template<size_t I> static constexpr color_type set_channel(color_type color, channel_type channel) {
        if constexpr (I < channel_count)
            color[I] = channel;
        return color;
    }

    template<typename... Ts> static constexpr color_type create(Ts... channels) {
        color_type color {};
        create_impl(color, channels..., std::make_index_sequence<std::min(sizeof...(Ts), channel_count)> {});
        return color;
    }

private:
    template<typename... Ts, size_t... Is> static constexpr void create_impl(color_type& out, Ts... channels, std::integer_sequence<size_t, Is...>) {
        auto ch_tup = std::make_tuple<Ts...>(std::move(channels)...);
        ((std::get<Is>(out) = std::get<Is>(ch_tup)), ...);
    }
};

#define GENERIC_UNSIGNED(_format, _channels, _repr)                                                                         \
    template<ColorSpace CSpace> struct ColorTraits<_format, CSpace> : public GenericColorTraits<CSpace, _channels, _repr> { \
        using base_type = GenericColorTraits<CSpace, _channels, _repr>;                                                     \
                                                                                                                            \
        static constexpr size_t channel_count = base_type::channel_count;                                                   \
        static constexpr std::array<size_t, channel_count> bit_widths = base_type::channel_count;                           \
        using channel_type = typename base_type::channel_type;                                                              \
        using color_type = typename base_type::color_type;                                                                  \
    }

GENERIC_UNSIGNED(ColorFormat::RGB8u, 3, uint8_t);
GENERIC_UNSIGNED(ColorFormat::RGBA8u, 4, uint8_t);
GENERIC_UNSIGNED(ColorFormat::BGRA8u, 4, uint8_t);
GENERIC_UNSIGNED(ColorFormat::RGBA16u, 4, uint16_t);
GENERIC_UNSIGNED(ColorFormat::RGB16u, 3, uint16_t);
GENERIC_UNSIGNED(ColorFormat::RGBA32u, 4, uint32_t);
GENERIC_UNSIGNED(ColorFormat::RGB32u, 3, uint32_t);
GENERIC_UNSIGNED(ColorFormat::RGBA32f, 4, float);
GENERIC_UNSIGNED(ColorFormat::RGB32f, 3, float);

#undef GENERIC_UNSIGNED

}
