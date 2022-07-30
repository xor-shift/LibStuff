#pragma once

#include <charconv>
#include <functional>

#include <Stuff/Serde/Serde.hpp>

namespace Stf::Serde::New {

template<typename It> struct BEncodeAdapter {
    using ser_sequence_type = BEncodeAdapter<It>&;
    using ser_tuple_type = ser_sequence_type;
    using ser_map_type = BEncodeAdapter<It>&;
    using ser_struct_type = ser_map_type;

    constexpr BEncodeAdapter(It it)
        : m_it(it) { }

    template<typename T> constexpr void serialize_primitive(T const& v) { m_it = serialize_primitive(m_it, v); }

    template<typename CharType, typename Traits = std::char_traits<CharType>> constexpr void serialize_char(CharType c) {
        m_it = serialize_char<CharType, Traits>(m_it, c);
    }

    template<typename CharType, typename Traits = std::char_traits<CharType>> constexpr void serialize_string(std::basic_string_view<CharType, Traits> str) {
        m_it = serialize_string(m_it, str);
    }

    constexpr ser_sequence_type serialize_sequence(size_t len) {
        *m_it++ = '[';
        begun_map_or_seq = true;
        return *this;
    }
    constexpr ser_tuple_type serialize_tuple(size_t len) { return serialize_sequence(len); }

    constexpr ser_map_type serialize_map(size_t len) {
        *m_it++ = '{';
        begun_map_or_seq = true;
        return *this;
    }
    constexpr ser_struct_type serialize_struct(size_t len, std::string_view name) { return serialize_map(len); }

    // required by sequence and tuple serializers
    template<typename T> constexpr void serialize_element(T const& v) {
        if (!begun_map_or_seq)
            *m_it++ = ',';
        begun_map_or_seq = false;

        serialize(v, *this);
    }

    // required by sequence and tuple serializers
    constexpr void end_sequence() { *m_it++ = ']'; }

    // required by map and struct serializers
    template<typename T> constexpr void serialize_key(T const& v) {
        if (!begun_map_or_seq)
            *m_it++ = ',';
        begun_map_or_seq = false;

        m_it = serialize_string(m_it, v);
        *m_it++ = ':';
    }

    // required by map and struct serializers
    template<typename T> constexpr void serialize_value(T const& v) { serialize(v, *this); }

    // required by map and struct serializers
    constexpr void end_map() { *m_it++ = '}'; }

protected:
    It m_it;
    bool begun_map_or_seq = false;

    template<typename T>
        requires(std::floating_point<T> || std::integral<T>)
    static constexpr It serialize_primitive(It it, T const& v) {
        std::array<char, 128> buffer;

        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), v, 10);

        if (ec != std::errc {})
            throw std::runtime_error("failed to convert primitive to chars");

        std::string_view sv(buffer.data(), std::distance(buffer.data(), ptr));
        return std::copy(sv.begin(), sv.end(), it);
    }

    template<typename CharType, typename Traits = std::char_traits<CharType>> static constexpr It serialize_char(It it, CharType c) {
        *it++ = '\'';
        *it++ = c;
        *it++ = '\'';
        return it;
    }

    template<typename CharType, typename Traits = std::char_traits<CharType>>
    static constexpr It serialize_string(It it, std::basic_string_view<CharType, Traits> str) {
        const auto guard_char = static_cast<CharType>(str.size() == 1 ? '\'' : '"');
        *it++ = guard_char;
        for (auto c : str)
            *it++ = c;
        *it++ = guard_char;
        return it;
    }
};

}
