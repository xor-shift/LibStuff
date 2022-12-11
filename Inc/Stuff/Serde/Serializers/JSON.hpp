#pragma once

#include <Stuff/Util/Hacks/Concepts.hpp>
#include <Stuff/Util/Hacks/Expected.hpp>
#include <Stuff/Util/Hacks/Try.hpp>

#include <charconv>
#include <concepts>
#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>

namespace Stf::Serde::JSON {

namespace Detail {
template<typename Stream> struct ListSerializer;
}

template<typename Stream> struct Serializer {
    Stream& stream;

    using ok_type = void;
    using error_type = std::string_view;

    using result_type = tl::expected<ok_type, error_type>;

    using array_serializer = Detail::ListSerializer<Stream>;
    using list_serializer = Detail::ListSerializer<Stream>;
    using tuple_serializer = Detail::ListSerializer<Stream>;

    template<std::floating_point T> constexpr result_type serialize_float(T v);

    constexpr result_type serialize_bool(bool b);

    template<std::integral T>
        requires(!std::is_same_v<T, bool>)
    constexpr result_type serialize_integral(T v);

    constexpr result_type serialize_bytes(std::span<uint8_t> v);

    template<typename Char, typename Traits = std::char_traits<Char>> constexpr result_type serialize_char(Char v);

    template<typename Char, typename Traits = std::char_traits<Char>>
    constexpr result_type serialize_str(std::basic_string_view<Char, Traits> str);

    template<size_t Size> constexpr tl::expected<array_serializer, error_type> serialize_array();

    constexpr tl::expected<list_serializer, error_type> serialize_list(std::optional<size_t> length);

    template<size_t Size> constexpr tl::expected<tuple_serializer, error_type> serialize_tuple();
};

namespace Detail {

template<typename Stream> struct ListSerializer {
    constexpr ListSerializer(Stream& stream)
        : m_stream(stream) {
        m_stream << '[';
    }

    using ok_type = void;
    using error_type = std::string_view;

    using result_type = tl::expected<ok_type, error_type>;

    template<typename T> constexpr tl::expected<void, std::string_view> serialize_element(T const& v) {
        if (!m_empty)
            m_stream << ", ";
        m_empty = false;

        Serializer<Stream> sub_ser { m_stream };
        Stf::serialize(sub_ser, v);

        return {};
    }

    constexpr void end() { m_stream << ']'; }

private:
    Stream& m_stream;
    bool m_empty = true;
};

}

template<typename Stream>
template<std::floating_point T>
constexpr typename Serializer<Stream>::result_type Serializer<Stream>::serialize_float(T v) {
    constexpr auto format = std::chars_format::general;
    std::array<char, 64> buf;
    const auto res = std::to_chars(buf.data(), buf.data() + buf.size(), v, format);

    if (res.ec != std::errc())
        return tl::unexpected("failed to format floating point value");

    std::span<char> fmt_chars(buf.data(), res.ptr);

    stream << std::string_view(fmt_chars);
    return {};
}

template<typename Stream>
constexpr typename Serializer<Stream>::result_type Serializer<Stream>::serialize_bool(bool b) {
    stream << (b ? "true" : "false");
    return {};
}

template<typename Stream>
template<std::integral T>
    requires(!std::is_same_v<T, bool>)
constexpr typename Serializer<Stream>::result_type Serializer<Stream>::serialize_integral(T v) {
    stream << v;
    return {};
}

template<typename Stream>
constexpr typename Serializer<Stream>::result_type Serializer<Stream>::serialize_bytes(std::span<uint8_t> v) {
    return {};
}

template<typename Stream>
template<typename Char, typename Traits>
constexpr typename Serializer<Stream>::result_type Serializer<Stream>::serialize_char(Char v) {
    return serialize_str<Char>({&v, &v + 1});
}

template<typename Stream>
template<typename Char, typename Traits>
constexpr typename Serializer<Stream>::result_type
Serializer<Stream>::serialize_str(std::basic_string_view<Char, Traits> str) {
    //TODO: escape strings
    stream << '"' << str << '"';
    return {};
}

template<typename Stream>
template<size_t Size>
constexpr tl::expected<typename Serializer<Stream>::array_serializer, typename Serializer<Stream>::error_type>
Serializer<Stream>::serialize_array() {
    return list_serializer { stream };
}

template<typename Stream>
constexpr tl::expected<typename Serializer<Stream>::list_serializer, typename Serializer<Stream>::error_type>
Serializer<Stream>::serialize_list(std::optional<size_t>) {
    return list_serializer { stream };
}

template<typename Stream>
template<size_t Size>
constexpr tl::expected<typename Serializer<Stream>::tuple_serializer, typename Serializer<Stream>::error_type>
Serializer<Stream>::serialize_tuple() {
    return list_serializer { stream };
}

}