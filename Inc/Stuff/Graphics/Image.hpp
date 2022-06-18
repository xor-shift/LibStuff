#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <span>

#include <Stuff/Maths/Maths.hpp>
#include <Stuff/Util/Util.hpp>

namespace Stf::Gfx {

using Color = std::array<uint8_t, 4>;

namespace Colors {

inline static constexpr Color black { 0, 0, 0, 255 };

}

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

template<typename Allocator = std::allocator<uint8_t>> struct Image {
    using allocator_type = Allocator;
    using color_type = Color;

    constexpr Image() = delete;

    constexpr Image(Image const& other, Allocator const& alloc = Allocator()) noexcept(noexcept(Allocator(alloc)))
        : m_allocator(alloc)
        , m_width(other.m_width)
        , m_height(other.m_height) {
        m_data = m_allocator.allocate(size());
        std::copy_n(other.m_data, size(), m_data);
    }

    constexpr Image(Image&& other, Allocator const& alloc = Allocator()) noexcept(noexcept(Allocator(alloc)))
        : m_allocator(alloc)
        , m_width(other.m_width)
        , m_height(other.m_height)
        , m_data(other.m_data) {
        other.m_data = nullptr;
    }

    constexpr Image(size_t width, size_t height, Allocator const& alloc = Allocator()) noexcept(noexcept(Allocator(alloc)))
        : m_allocator(alloc)
        , m_width(width)
        , m_height(height) {
        m_data = m_allocator.allocate(size());
    }

    constexpr Image(size_t width, size_t height, Color fill_color, Allocator const& alloc = Allocator()) noexcept(noexcept(Allocator(alloc)))
        : m_allocator(alloc)
        , m_width(width)
        , m_height(height) {
        m_data = m_allocator.allocate(size());
        fill(fill_color);
    }

    ~Image() noexcept(noexcept(m_allocator.deallocate(m_data, m_width* m_height))) {
        if (m_data != nullptr)
            m_allocator.deallocate(m_data, m_width * m_height);
    }

    constexpr void fill(Color fill_color) {
        uint32_t col = std::bit_cast<uint32_t>(fill_color);
        auto* const out_ptr = reinterpret_cast<uint32_t*>(m_data);
        const auto out_size = size() / sizeof(uint32_t);
        std::fill_n(out_ptr, out_size, col);
    }

    constexpr Vector<size_t, 2> dimensions() const { return vector(m_width, m_height); }

    /// Raw data access functions
    constexpr size_t size() const { return m_width * m_height * 4; }
    /// Raw data access functions
    constexpr uint8_t* data() const { return m_data; }
    /// Raw data access functions
    constexpr uint8_t* begin() const { return data(); }
    /// Raw data access functions
    constexpr uint8_t* end() const { return data() + size(); }

    constexpr size_t pixel_count() const { return m_width * m_height; }
    constexpr color_type get_pixel(size_t i) const {
        const auto* const p = m_data + i * 4;
        return color_type { p[0], p[1], p[2], p[3] };
    }
    constexpr void set_pixel(size_t i, color_type color) const { std::ranges::copy(color, m_data + i * 4); }

    constexpr ColorFormat color_format() const { return m_color_format_hint; }
    constexpr ColorSpace color_space() const { return m_color_space_hint; }

private:
    allocator_type m_allocator;

    ColorFormat m_color_format_hint = ColorFormat::RGBA8u;
    ColorSpace m_color_space_hint = ColorSpace::SRGB;

    size_t m_width;
    size_t m_height;
    uint8_t* m_data;
};

}

#include "./Image/QoI.ipp"
// #include "./Image/TGA.ipp"

namespace Gfx = Stf::Gfx;
