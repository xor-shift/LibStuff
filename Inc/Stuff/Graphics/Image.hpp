#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <span>

#include <Stuff/Maths/BLAS/Vector.hpp>

#include "./Traits.hpp"

namespace Stf::Gfx {

template<ColorFormat Format, ColorSpace Space, typename Traits = ColorTraits<Format, Space>, typename Allocator = std::allocator<typename Traits::color_type>>
struct NewImage {
    using traits = Traits;
    using channel_type = typename traits::channel_type;
    using color_type = typename traits::color_type;

    constexpr NewImage(Allocator const& allocator = Allocator())
     : m_allocator(allocator) {}

    constexpr ~NewImage() noexcept {
        destroy();
    }

    /// Resets the image data and creates a new image with the specified
    /// information. Can be called multiple times.
    constexpr void create(Vector<size_t, 2> dimensions) {
        destroy();
        m_dimensions = dimensions;
        m_data = m_allocator.allocate(pixel_count());
    }

    /// Resets the image
    constexpr void destroy() noexcept {
        if (m_data != nullptr) {
            m_allocator.deallocate(m_data, pixel_count());
            m_dimensions = { 0, 0 };
        }
    }

    inline const uint8_t* data() const { return reinterpret_cast<const uint8_t*>(m_data); }
    inline uint8_t* data() { return reinterpret_cast<uint8_t*>(m_data); }
    constexpr size_t size() { return pixel_count() * sizeof(color_type); }

    constexpr Vector<size_t, 2> dimensions() const { return m_dimensions; }

    constexpr std::span<color_type> pixels() { return { m_data, pixel_count() }; }
    constexpr std::span<const color_type> pixels() const { return { m_data, pixel_count() }; }
    constexpr size_t pixel_count() const { return m_dimensions[0] * m_dimensions[1]; }

    constexpr void fill(color_type color) { std::fill_n(m_data, pixel_count(), color); }

    constexpr size_t coords_to_index(Vector<size_t, 2> coords) const { return coords[0] + coords[1] * m_dimensions[0]; }

    constexpr color_type& operator[](size_t index) & { return m_data[index]; }
    constexpr color_type operator[](size_t index) const& { return m_data[index]; }
    constexpr color_type&& operator[](size_t index) && { return m_data[index]; }
    constexpr color_type& operator[](Vector<size_t, 2> coords) & { return (*this)[coords_to_index(coords)]; }
    constexpr color_type operator[](Vector<size_t, 2> coords) const& { return (*this)[coords_to_index(coords)]; }
    constexpr color_type&& operator[](Vector<size_t, 2> coords) && { return (*this)[coords_to_index(coords)]; }

private:
    Allocator m_allocator;

    Vector<size_t, 2> m_dimensions { 0, 0 };

    color_type* m_data { nullptr };
};

using Color = std::array<uint8_t, 4>;

namespace Colors {

inline static constexpr Color black { 0, 0, 0, 255 };

}

template<typename Allocator = std::allocator<uint8_t>> struct Image {
    using allocator_type = Allocator;
    using color_type = Color;

    constexpr Image(Allocator const& alloc = Allocator()) noexcept
        : m_allocator(alloc) { }

    constexpr Image(Image const& other, Allocator const& alloc = Allocator()) noexcept(noexcept(Allocator(alloc)))
        : m_allocator(alloc) {
        create(other.m_width, other.m_height);
        std::copy_n(other.m_data, m_capacity, m_data);
    }

    constexpr Image(Image&& other, Allocator const& alloc = Allocator()) noexcept(noexcept(Allocator(alloc)))
        : m_allocator(alloc)
        , m_width(other.m_width)
        , m_height(other.m_height)
        , m_data(other.m_data) {
        other.m_data = nullptr;
        other.reset();
    }

    constexpr Image(size_t width, size_t height, Allocator const& alloc = Allocator()) noexcept(noexcept(Allocator(alloc)))
        : m_allocator(alloc) {
        create(width, height);
    }

    constexpr Image(size_t width, size_t height, Color fill_color, Allocator const& alloc = Allocator()) noexcept(noexcept(Allocator(alloc)))
        : m_allocator(alloc)
        , m_width(width)
        , m_height(height) {
        create(width, height);
        fill(fill_color);
    }

    constexpr ~Image() noexcept(noexcept(m_allocator.deallocate(m_data, m_width* m_height))) { reset(); }

    constexpr Image& operator=(Image&& other) {
        m_allocator = other.m_allocator;
        m_color_format_hint = other.m_color_format_hint;
        m_color_space_hint = other.m_color_space_hint;
        m_width = other.m_width;
        m_height = other.m_height;
        m_data = other.m_data;
        other.m_data = nullptr;
        other.reset();
        return *this;
    }

    constexpr Image& operator=(Image const& other) {
        m_allocator = other.m_allocator;
        m_color_format_hint = other.m_color_format_hint;
        m_color_space_hint = other.m_color_space_hint;

        create(other.m_width, other.m_height);
        std::copy_n(other.m_data, m_capacity, m_data);

        return *this;
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
    constexpr auto set_pixel(size_t x, size_t y, color_type color) const { return set_pixel(x + y * m_width, color); }
    constexpr auto get_pixel(size_t x, size_t y) const { return get_pixel(x + y * m_width); }

    constexpr ColorFormat color_format() const { return m_color_format_hint; }
    constexpr ColorSpace color_space() const { return m_color_space_hint; }

    constexpr void reset() {
        m_width = 0;
        m_height = 0;

        if (m_data != nullptr)
            m_allocator.deallocate(m_data, m_capacity);

        m_capacity = 0;
        m_data = nullptr;
    }

    constexpr void create(size_t width, size_t height) {
        reset();

        m_width = width;
        m_height = height;
        m_capacity = width * height * 4;
        m_data = m_allocator.allocate(m_capacity);
    }

    constexpr void resize(size_t width, size_t height) {
        Image image(width, height, m_allocator);
        *this = image;
    }

private:
    allocator_type m_allocator;

    ColorFormat m_color_format_hint = ColorFormat::RGBA8u;
    ColorSpace m_color_space_hint = ColorSpace::SRGB;

    size_t m_width = 0;
    size_t m_height = 0;
    size_t m_capacity = 0;
    uint8_t* m_data = nullptr;
};

}

#include "./Image/QoI.ipp"
// #include "./Image/TGA.ipp"

namespace Gfx {

using namespace Stf::Gfx;

}
