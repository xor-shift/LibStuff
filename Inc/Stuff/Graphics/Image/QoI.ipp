#pragma once

#include <iterator>
#include <span>

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
    uint8_t magic[4];
    uint32_t width;
    uint32_t height;
    uint8_t channels;
    uint8_t colorspace;

    template<std::input_iterator IIter> static constexpr Error<RawHeader, std::string_view> from_bytes(IIter&& begin, IIter end) {
        std::array<uint8_t, 4> magic_bytes;
        unwrap_or_return(magic_bytes, get_bytes<4>(std::forward<IIter>(begin), end), "Failed to read magic");
        std::array<uint8_t, 4> width_bytes;
        unwrap_or_return(width_bytes, get_bytes<4>(std::forward<IIter>(begin), end), "Failed to read width");
        std::array<uint8_t, 4> height_bytes;
        unwrap_or_return(height_bytes, get_bytes<4>(std::forward<IIter>(begin), end), "Failed to read height");
        std::array<uint8_t, 2> info_bytes;
        unwrap_or_return(info_bytes, get_bytes<2>(std::forward<IIter>(begin), end), "Failed to read image information");

        RawHeader header;
        std::copy_n(magic_bytes.begin(), 4, header.magic);
        header.width = convert_endian(std::bit_cast<uint32_t>(width_bytes), std::endian::big);
        header.height = convert_endian(std::bit_cast<uint32_t>(height_bytes), std::endian::big);
        header.channels = info_bytes[0];
        header.colorspace = info_bytes[1];

        return header;
    }

    constexpr std::array<uint8_t, 14> to_bytes() const {
        std::array<uint8_t, 14> ret;

        const auto w_bytes = std::bit_cast<std::array<char, 4>>(Stf::convert_endian(width, std::endian::native, std::endian::big));
        const auto h_bytes = std::bit_cast<std::array<char, 4>>(Stf::convert_endian(height, std::endian::native, std::endian::big));

        std::copy_n(magic, 4, ret.begin());
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

    template<std::input_iterator IIter> static constexpr Error<Header, std::string_view> from_bytes(IIter&& begin, IIter end) {
        RawHeader raw_header;
        unwrap_or_return(raw_header, RawHeader::from_bytes(std::forward<IIter>(begin), end), res.error());

        Header header;
        unwrap_or_return(header, from_raw(raw_header), res.error());

        return header;
    }

    static constexpr Error<Header, std::string_view> from_raw(RawHeader const& raw_header) {
        constexpr std::array<char, 4> expected_magic { 'q', 'o', 'i', 'f' };
        std::array<char, 4> given_magic {};
        std::copy_n(raw_header.magic, 4, given_magic.begin());

        if (expected_magic != given_magic) {
            return "Bad header magic";
        }

        if (raw_header.width == 0 || raw_header.height == 0) {
            return "Bad header dimensions";
        }

        if (raw_header.channels != 3 && raw_header.channels != 4) {
            return "Bad header color channel specifier";
        }

        if (raw_header.colorspace != 0 && raw_header.colorspace != 1) {
            return "Bad header colorspace specifier";
        }

        return Header {
            .dims = Stf::vector(raw_header.width, raw_header.height),
            .channels = static_cast<Channels>(raw_header.channels),
            .colorspace = static_cast<ColorSpace>(raw_header.colorspace),
        };
    }

    template<typename Allocator = std::allocator<uint8_t>> static constexpr Error<Header, std::string_view> from_image(Gfx::Image<Allocator> const& image) {
        Header header;

        const auto [w, h] = image.dimensions();

        if (w > std::numeric_limits<uint32_t>::max())
            return "Image is too wide";

        if (h > std::numeric_limits<uint32_t>::max())
            return "Image is too high";

        if (w == 0)
            return "Image has zero width";

        if (h == 0)
            return "Image has zero height";

        header.dims = Stf::vector<uint32_t>(w, h);

        switch (image.color_format()) {
        case ColorFormat::RGBA8u: header.channels = Channels::RGBA; break;
        case ColorFormat::RGB8u: header.channels = Channels::RGB; break;
        default: return "Image has unsupported color format, only RGBA8u and RGB8u are supported";
        }

        switch (image.color_space()) {
        case Gfx::ColorSpace::SRGB: header.colorspace = ColorSpace::SRGBLinearAlpha; break;
        case Gfx::ColorSpace::Linear: header.colorspace = ColorSpace::Linear; break;
        default: return "Image has unsupported colorspace, only SRGB and Linear are supported";
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

template<typename Allocator = std::allocator<uint8_t>>
constexpr std::optional<std::string_view> decode_into(std::span<const uint8_t> payload, Gfx::Image<Allocator>& image) {
    QoIDecoder state {
        .out_beg = image.data(),
        .out_end = image.data() + image.size(),
    };

    for (auto in_p = payload.begin(); in_p != payload.end();) {
        std::array<uint8_t, 4> extra_data = state.last_seen;

        uint8_t leading = *in_p++;
        const auto type = block_type(leading);
        const auto extra_size = extra_data_size(type);
        std::copy_n(in_p, extra_size, extra_data.begin());
        in_p += extra_size;

        if (!state.process_block(leading, extra_data))
            return "Overflow while reading QoI blocks";
    }

    if (state.it != state.out_end)
        return "Insufficient data (Incomplete image)";

    return std::nullopt;
}

template<typename Allocator = std::allocator<uint8_t>>
constexpr Error<Gfx::Image<Allocator>, std::string_view> decode(std::span<const uint8_t> encoded, Allocator const& allocator = Allocator()) {
    if (encoded.size() < 14 + 8)
        return "Insufficient data";

    const auto header_span = encoded.subspan(0, 14);
    const auto payload_span = encoded.subspan(14, encoded.size() - 14 - 8);
    const auto ending_span = encoded.subspan(encoded.size() - 8, 8);

    Header header;
    unwrap_or_return(header, Header::from_bytes(header_span.begin(), header_span.end()), res.error());

    Gfx::Image<Allocator> image(header.dims[0], header.dims[1], allocator);

    if (auto res = decode_into(payload_span, image); res)
        return *res;

    if (!std::all_of(ending_span.begin(), ending_span.end() - 1, [](auto v) { return v == 0x00; }) || ending_span.back() != 0x01)
        return "Bad ending bytes";

    return image;
}


template<typename Allocator = std::allocator<uint8_t>, std::input_iterator IIter>
constexpr Error<IIter, std::string_view> decode_into(IIter it, IIter end, Gfx::Image<Allocator>& image) {
    QoIDecoder state {
        .out_beg = image.data(),
        .out_end = image.data() + image.size(),
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
                return "Insufficient data (while reading a block's extra data)";
            extra_data[i] = *it++;
        }

        if (!state.process_block(leading, extra_data))
            return "Overflow while reading QoI blocks";
    }

    return it;
}

template<typename Allocator = std::allocator<uint8_t>, std::input_iterator IIter>
constexpr Error<Gfx::Image<Allocator>, std::string_view> decode(IIter begin, IIter end, Allocator const& allocator = Allocator()) {
    IIter it = begin;

    Header header;
    unwrap_or_return(header, Header::from_bytes(std::forward<IIter>(it), end), res.error());

    Gfx::Image<Allocator> image(header.dims[0], header.dims[1], allocator);

    unwrap_or_return(it, decode_into(it, end, image), res.error());

    for (size_t i = 0; i < 7; i++) {
        if (it == end)
            return "Insufficient data (while reading ending bytes)";
        if (*it++ != 0)
            return "Bad ending bytes";
    }

    return image;
}

template<typename Allocator = std::allocator<uint8_t>, std::output_iterator<uint8_t> OIter>
constexpr void encode(OIter out_beg, Gfx::Image<Allocator> const& image) {
    Header header;
    unwrap_or_return(header, Header::from_image(image), );
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

    /*const auto emit_run = [&](color_type pixel) {

        if (run_length >= 60) {
            do_emit_run();
        }

        if (pixel == last_seen) {
            ++run_length;
            return true;
        } else if (run_length != 0) {
            do_emit_run();
            return false;
        }

        return false;
    };*/

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

        if (run_length >= 62)
            do_emit_run();

        if (cur == last_seen) {
            ++run_length;
            continue;
        } else if (run_length != 0) {
            do_emit_run();
        }

        if (emit_index(cur, cur_hash))
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
}

}

namespace Stf::Gfx::Formats::QoI {

using Gfx::Detail::Image::QoI::decode;
using Gfx::Detail::Image::QoI::decode_into;
using Gfx::Detail::Image::QoI::encode;

}
