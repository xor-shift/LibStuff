#include <Stuff/Util/Hacks/Try.hpp>

#include <Stuff/Files/Format.hpp>
#include <Stuff/Maths/Bit.hpp>

#include <concepts>
#include <cstddef>

// types etc.
namespace Stf::Gfx::Detail::Image::QoI {

template<size_t N, std::input_iterator IIter> constexpr std::optional<std::array<uint8_t, N>> get_bytes(IIter&& it, IIter end) {
    static_assert(N > 0, "amount of bytes read should be greater than 0");

    std::array<uint8_t, N> buf;
    auto o_it = buf.begin();

    while (it != end) {
        *o_it++ = *it++;

        if (o_it == buf.end())
            return buf;
    }

    return std::nullopt;
}

template<typename T> using Color = std::array<T, 4>;

enum class BlockType {
    RGB,
    RGBA,
    Index,
    Diff,
    Luma,
    Run,
};

constexpr BlockType block_type(uint8_t b) {
    if (b == 0xFF)
        return BlockType::RGBA;
    if (b == 0xFE)
        return BlockType::RGB;

    switch (b >> 6) {
    case 0: return BlockType::Index;
    case 1: return BlockType::Diff;
    case 2: return BlockType::Luma;
    case 3: return BlockType::Run;
    default: std::unreachable();
    }
}

constexpr size_t extra_data_size(BlockType type) {
    switch (type) {
    case BlockType::RGB: return 3;
    case BlockType::RGBA: return 4;
    case BlockType::Index: return 0;
    case BlockType::Diff: return 0;
    case BlockType::Luma: return 1;
    case BlockType::Run: return 0;
    }
    std::unreachable();
}

enum class Channels {
    RGB = 3,
    RGBA = 4,
};

enum class ColorSpace {
    SRGBLinearAlpha = 0,
    Linear = 1,
};

struct RawHeader {
    std::array<uint8_t, 4> magic;
    uint32_t width;
    uint32_t height;
    uint8_t channels;
    uint8_t colorspace;

    template<std::input_iterator IIter> static constexpr tl::expected<RawHeader, std::string_view> from_bytes(IIter&& begin, IIter end) {
        using namespace FFormat;

        constexpr auto header_format = FFormat::group_field()
            << name_field(make_array_field<4>(primitive_field<Primitive::U8>()), "magic") << name_field(primitive_field<Primitive::U32BE>(), "width")
            << name_field(primitive_field<Primitive::U32BE>(), "height") << name_field(primitive_field<Primitive::U8>(), "channels")
            << name_field(primitive_field<Primitive::U8>(), "colorspace");

        using header_format_type = decltype(header_format);

        header_format_type::representation_type header_tuple;
        auto res = header_format.template decode(begin, end, header_tuple);

        if (!res)
            return tl::unexpected { "Failed to read header" };

        begin = *res;

        return RawHeader {
            .magic = std::get<0>(header_tuple),
            .width = std::get<1>(header_tuple),
            .height = std::get<2>(header_tuple),
            .channels = std::get<3>(header_tuple),
            .colorspace = std::get<4>(header_tuple),
        };
    }

    constexpr std::array<uint8_t, 14> to_bytes() const {
        std::array<uint8_t, 14> ret;

        const auto w_bytes = std::bit_cast<std::array<char, 4>>(Stf::convert_endian(width, std::endian::native, std::endian::big));
        const auto h_bytes = std::bit_cast<std::array<char, 4>>(Stf::convert_endian(height, std::endian::native, std::endian::big));

        std::copy_n(magic.begin(), 4, ret.begin());
        std::ranges::copy(w_bytes, ret.begin() + 4);
        std::ranges::copy(h_bytes, ret.begin() + 8);
        ret[12] = channels;
        ret[13] = colorspace;

        return ret;
    }
};

struct Header {
    Stf::Vector<uint32_t, 2> dims;
    Channels channels;
    ColorSpace colorspace;

    template<std::input_iterator IIter> static constexpr tl::expected<Header, std::string_view> from_bytes(IIter&& begin, IIter end) {
        const auto raw_header = TRYX(RawHeader::from_bytes(std::forward<IIter>(begin), end));
        const auto header = TRYX(from_raw(raw_header));
        return header;
    }

    static constexpr tl::expected<Header, std::string_view> from_raw(RawHeader const& raw_header) {
        constexpr std::array<char, 4> expected_magic { 'q', 'o', 'i', 'f' };
        std::array<char, 4> given_magic {};
        std::copy_n(raw_header.magic.begin(), 4, given_magic.begin());

        if (expected_magic != given_magic) {
            return tl::unexpected { "Bad header magic" };
        }

        if (raw_header.width == 0 || raw_header.height == 0) {
            return tl::unexpected { "Bad header dimensions" };
        }

        if (raw_header.channels != 3 && raw_header.channels != 4) {
            return tl::unexpected { "Bad header color channel specifier" };
        }

        if (raw_header.colorspace != 0 && raw_header.colorspace != 1) {
            return tl::unexpected { "Bad header colorspace specifier" };
        }

        return Header {
            .dims = Stf::vector(raw_header.width, raw_header.height),
            .channels = static_cast<Channels>(raw_header.channels),
            .colorspace = static_cast<ColorSpace>(raw_header.colorspace),
        };
    }

    template<typename Allocator = std::allocator<uint8_t>> static constexpr tl::expected<Header, std::string_view> from_image(Gfx::Image<Allocator> const& image) {
        Header header;

        const auto [w, h] = image.dimensions();

        if (w > std::numeric_limits<uint32_t>::max())
            return tl::unexpected { "Image is too wide" };

        if (h > std::numeric_limits<uint32_t>::max())
            return tl::unexpected { "Image is too high" };

        if (w == 0)
            return tl::unexpected { "Image has zero width" };

        if (h == 0)
            return tl::unexpected { "Image has zero height" };

        header.dims = Stf::vector<uint32_t>(w, h);

        switch (image.color_format()) {
        case ColorFormat::RGBA8u: header.channels = Channels::RGBA; break;
        case ColorFormat::RGB8u: header.channels = Channels::RGB; break;
        default: return tl::unexpected { "Image has unsupported color format, only RGBA8u and RGB8u are supported" };
        }

        switch (image.color_space()) {
        case Gfx::ColorSpace::SRGB: header.colorspace = ColorSpace::SRGBLinearAlpha; break;
        case Gfx::ColorSpace::Linear: header.colorspace = ColorSpace::Linear; break;
        default: return tl::unexpected { "Image has unsupported colorspace, only SRGB and Linear are supported" };
        }

        return header;
    }

    constexpr RawHeader raw() const {
        return {
            .magic = { 'q', 'o', 'i', 'f' },
            .width = dims[0],
            .height = dims[1],
            .channels = static_cast<uint8_t>(channels),
            .colorspace = static_cast<uint8_t>(colorspace),
        };
    }

    constexpr std::array<uint8_t, 14> to_bytes() const { return raw().to_bytes(); }
};

template<typename T, T RMult, T GMult, T BMult, T AMult, T Mod> struct ColorMap {
    using color_type = Color<T>;

    static constexpr T hash(color_type c) noexcept {
        // TODO: find a way to explicitly multiply and add with wrapping.

        const auto r = c[0] * RMult;
        const auto g = c[1] * GMult;
        const auto b = c[2] * BMult;
        const auto a = c[3] * AMult;

        return (r + g + b + a) % Mod;
    }

    constexpr color_type& operator[](uint8_t index) & { return m_data[index]; }
    constexpr color_type const& operator[](uint8_t index) const& { return m_data[index]; }

    constexpr color_type& operator[](color_type color) & { return m_data[hash(color)]; }
    constexpr color_type const& operator[](color_type color) const& { return m_data[hash(color)]; }

private:
    std::array<color_type, Mod> m_data {};
};

using QoIColorMap = ColorMap<uint8_t, 3, 5, 7, 11, 64>;

}

namespace Stf::Gfx::Detail::Image::QoI {

struct QoIDecoder {
    using color_type = QoIColorMap::color_type;

    uint8_t* const out_beg = nullptr;
    uint8_t* const out_end = nullptr;
    uint8_t* it = out_beg;
    QoIColorMap map {};
    color_type last_seen { 0, 0, 0, 255 };

    constexpr bool has_remaining(size_t pixels) { return std::distance(it, out_end) >= pixels * sizeof(color_type); }

    template<bool CheckRemaining = true, bool Save = true> constexpr std::conditional_t<CheckRemaining, bool, void> rgba(color_type color) {
        if constexpr (CheckRemaining)
            if (!has_remaining(1))
                return false;

        it = std::copy_n(color.begin(), 4, it);
        last_seen = color;
        if constexpr (Save)
            map[color] = color;

        if constexpr (CheckRemaining)
            return true;
    }

    template<bool CheckRemaining = true> constexpr auto index(uint8_t index) { return rgba<CheckRemaining, false>(map[index]); }

    template<bool CheckRemaining = true> constexpr auto diff(int8_t dr, int8_t dg, int8_t db) {
        const auto r = static_cast<uint8_t>(last_seen[0] + dr - 2);
        const auto g = static_cast<uint8_t>(last_seen[1] + dg - 2);
        const auto b = static_cast<uint8_t>(last_seen[2] + db - 2);

        return rgba<CheckRemaining>({ r, g, b, last_seen[3] });
    }

    template<bool CheckRemaining = true> constexpr auto luma(int8_t dg, int8_t drdg, int8_t dbdg) {
        dg -= 32;
        const auto dr = drdg + dg - 8;
        const auto db = dbdg + dg - 8;

        const auto r = static_cast<uint8_t>(last_seen[0] + dr);
        const auto g = static_cast<uint8_t>(last_seen[1] + dg);
        const auto b = static_cast<uint8_t>(last_seen[2] + db);

        return rgba<CheckRemaining>({ r, g, b, last_seen[3] });
    }

    template<bool CheckRemaining = true> constexpr std::conditional_t<CheckRemaining, bool, void> run(uint8_t run_length) {
        if constexpr (CheckRemaining) {
            if (!has_remaining(run_length + 1))
                return false;
        }

        for (uint8_t i = 0; i <= run_length; i++)
            it = std::copy_n(last_seen.begin(), 4, it);

        if constexpr (CheckRemaining)
            return true;
    }

    template<bool CheckRemaining = true> constexpr bool process_block(uint8_t leading, std::array<uint8_t, 4> extra_data) {
        const auto type = block_type(leading);

        bool no_overflow;
        switch (type) {
        case BlockType::RGB: [[fallthrough]];
        case BlockType::RGBA: no_overflow = rgba<CheckRemaining>(extra_data); break;
        case BlockType::Index: no_overflow = index<CheckRemaining>(leading & 0x3F); break;
        case BlockType::Diff: no_overflow = diff<CheckRemaining>((leading >> 4) & 3, (leading >> 2) & 3, leading & 3); break;
        case BlockType::Luma: no_overflow = luma<CheckRemaining>(leading & 0x3F, (extra_data[0] >> 4) & 0xF, extra_data[0] & 0xF); break;
        case BlockType::Run: no_overflow = run<CheckRemaining>(leading & 0x3F); break;
        }

        return no_overflow;
    }
};

template<std::input_iterator IIter> constexpr tl::expected<IIter, std::string_view> decode_payload(IIter it, IIter end, std::span<uint8_t> out_image_data) {
    QoIDecoder state {
        .out_beg = out_image_data.data(),
        .out_end = out_image_data.data() + out_image_data.size(),
    };

    while (state.it != state.out_end) {
        std::array<uint8_t, 4> extra_data = state.last_seen;

        if (it == end)
            break;

        uint8_t leading = *it++;
        const auto type = block_type(leading);
        const auto extra_size = extra_data_size(type);

        for (size_t i = 0; i < extra_size; i++) {
            if (it == end)
                return tl::unexpected { "Insufficient data (while reading a block's extra data)" };
            extra_data[i] = *it++;
        }

        if (!state.process_block(leading, extra_data))
            return tl::unexpected { "Overflow while reading QoI blocks" };
    }

    return it;
}

template<typename Allocator = std::allocator<uint8_t>, std::input_iterator IIter>
constexpr tl::expected<IIter, std::string_view> decode(IIter begin, IIter end, Gfx::Image<Allocator>& image, std::optional<Header> given_header = std::nullopt) {
    IIter it = begin;

    Header header;
    if (given_header)
        header = *given_header;
    else
        header = TRYX(Header::from_bytes(std::forward<IIter>(it), end));

    const auto [i_w, i_h] = image.dimensions();
    const auto [h_w, h_h] = header.dims;

    if (i_w != h_w || i_h != h_h)
        return tl::unexpected { "Image metadata does not match header metadata" };

    it = TRYX(decode_payload(it, end, image));

    for (size_t i = 0; i < 7; i++) {
        if (it == end)
            return tl::unexpected { "Insufficient data (while reading ending bytes)" };
        if (*it++ != 0)
            return tl::unexpected { "Bad ending bytes" };
    }

    return it;
}

template<typename Allocator = std::allocator<uint8_t>, std::input_iterator IIter>
constexpr tl::expected<Gfx::Image<Allocator>, std::string_view> decode(IIter begin, IIter end, Allocator const& allocator = Allocator()) {
    IIter it = begin;

    const auto header = TRYX(Header::from_bytes(std::forward<IIter>(it), end));

    Gfx::Image<Allocator> image(header.dims[0], header.dims[1], allocator);
    std::ignore = TRYX(decode(it, end, image, header));

    return image;
}

template<typename Allocator = std::allocator<uint8_t>, std::output_iterator<uint8_t> OIter>
constexpr tl::expected<OIter, std::string_view> encode(OIter out_beg, Gfx::Image<Allocator> const& image) {
    const auto header = TRYX(Header::from_image(image));
    const auto header_bytes = header.to_bytes();

    using color_type = QoIColorMap::color_type;

    QoIColorMap map {};
    color_type last_seen { 0, 0, 0, 255 };
    uint8_t run_length = 0;
    auto out_it = std::copy(header_bytes.cbegin(), header_bytes.cend(), out_beg);

    const auto do_emit_run = [&] {
        *out_it++ = ((run_length - 1) & 0x3F) | 0xC0;
        run_length = 0;
    };

    const auto emit_run = [&](color_type pixel) {
        if (run_length >= 62)
            do_emit_run();

        if (pixel == last_seen) {
            ++run_length;
            return true;
        } else if (run_length != 0) {
            do_emit_run();
        }

        return false;
    };

    const auto emit_rgba = [&](color_type pixel) {
        bool emit_alpha = pixel[3] != last_seen[3];

        if (emit_alpha)
            *out_it++ = 0xFF;
        else
            *out_it++ = 0xFE;

        *out_it++ = pixel[0];
        *out_it++ = pixel[1];
        *out_it++ = pixel[2];
        if (emit_alpha)
            *out_it++ = pixel[3];
    };

    const auto emit_index = [&](color_type pixel, uint8_t hash) {
        if (map[hash] != pixel)
            return false;
        *out_it++ = hash & 0x3F;
        return true;
    };

    const auto emit_diff = [&](color_type pixel) -> bool {
        if (pixel[3] != last_seen[3])
            return false;

        std::array<int, 3> deltas {
            pixel[0] - last_seen[0],
            pixel[1] - last_seen[1],
            pixel[2] - last_seen[2],
        };

        for (auto& v : deltas)
            v += 2;

        if (!std::ranges::all_of(deltas, [](int v) { return 3 >= v && v >= 0; }))
            return false;

        deltas[0] <<= 4;
        deltas[1] <<= 2;

        *out_it++ = 0x40 | deltas[0] | deltas[1] | deltas[2];
        return true;
    };

    const auto emit_luma = [&](color_type pixel) -> bool {
        if (pixel[3] != last_seen[3])
            return false;

        const auto dg = pixel[1] - last_seen[1];
        const auto dr = pixel[0] - last_seen[0] - dg;
        const auto db = pixel[2] - last_seen[2] - dg;

        if (-32 > dg || dg > 31 || -8 > dr || dr > 7 || -8 > db || db > 7)
            return false;

        const auto dro = dr + 8;
        const auto dgo = dg + 32;
        const auto dbo = db + 8;

        *out_it++ = 0x80 | dgo;
        *out_it++ = (dro << 4) | dbo;

        return true;
    };

    for (size_t i = 0; i < image.pixel_count(); i++) {
        const auto cur = image.get_pixel(i);
        const auto cur_hash = QoIColorMap::hash(cur);

        if (emit_run(cur))
            std::ignore = std::ignore;
        else if (emit_index(cur, cur_hash))
            std::ignore = std::ignore;
        else if (emit_diff(cur))
            std::ignore = std::ignore;
        else if (emit_luma(cur))
            std::ignore = std::ignore;
        else
            emit_rgba(cur);

        last_seen = cur;
        map[cur_hash] = last_seen;
    }

    if (run_length != 0)
        do_emit_run();

    out_it = std::fill_n(out_it, 7, 0);
    *out_it++ = 1;

    return out_it;
}

}

namespace Stf::Gfx::Formats::QoI {

using Gfx::Detail::Image::QoI::decode;
using Gfx::Detail::Image::QoI::encode;

}
