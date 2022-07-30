#pragma once

#include <Stuff/Util/Hacks/Concepts.hpp>
#include <Stuff/Util/Hacks/Expected.hpp>
#include <Stuff/Util/Hacks/Try.hpp>

#include <charconv>
#include <functional>

#include <Stuff/Serde/Serde.hpp>

namespace Stf::Serde::New {

namespace Detail {

template<typename It> struct JSONSerializer {
    using ser_sequence_type = JSONSerializer<It>&;
    using ser_tuple_type = ser_sequence_type;
    using ser_map_type = JSONSerializer<It>&;
    using ser_struct_type = ser_map_type;

    constexpr JSONSerializer(It it)
        : m_it(it) { }

    template<typename T> constexpr void serialize_bool(T v) {
        std::string_view view = (bool)v ? "true" : "false";
        m_it = std::copy(view.cbegin(), view.cend(), m_it);
    }

    template<typename T> constexpr void serialize_primitive(T v) { m_it = serialize_primitive(m_it, v); }
    constexpr void serialize_u128(__uint128_t v) { serialize_primitive(static_cast<uint64_t>(v)); }
    constexpr void serialize_i128(__int128_t v) { serialize_primitive(static_cast<int64_t>(v)); }

    template<typename CharType, typename Traits = std::char_traits<CharType>> constexpr void serialize_char(CharType c) {
        m_it = serialize_char<CharType, Traits>(m_it, c);
    }

    template<typename CharType, typename Traits = std::char_traits<CharType>> constexpr void serialize_string(std::basic_string_view<CharType, Traits> str) {
        m_it = serialize_string(m_it, str);
    }

    // empty struct or variant
    constexpr void serialize_unit() {
        std::string_view view = "null";
        m_it = std::copy(view.cbegin(), view.cend(), m_it);
    }

    // std::nullopt
    constexpr void serialize_nullopt() { serialize_unit(); }

    // std::optional that has a value
    template<typename T> constexpr void serialize_some(T const& v) { serialize(v, *this); }

    // std::monostate of a std::variant
    constexpr void serialize_monostate() { serialize_unit(); }

    template<typename T> constexpr void serialize_variant(T const& v, size_t variant_index) { serialize(v, *this); }

    constexpr ser_sequence_type serialize_sequence(size_t len) {
        *m_it++ = '[';
        begun_map_or_seq = true;
        return *this;
    }
    constexpr ser_tuple_type serialize_tuple(size_t len) { return serialize_sequence(len); }

    constexpr ser_map_type serialize_map(size_t len) {
        //++cur_indent;
        *m_it++ = '{';
        begun_map_or_seq = true;
        return *this;
    }
    constexpr ser_struct_type serialize_struct(size_t len, std::string_view) { return serialize_map(len); }

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

        emit_newline();
        emit_indent();

        m_it = serialize_string(m_it, v);
        *m_it++ = ':';
        //*m_it++ = ' ';
    }

    // required by map and struct serializers
    template<typename T> constexpr void serialize_value(T const& v) { serialize(v, *this); }

    // required by map and struct serializers
    constexpr void end_map() {
        // emit_newline();
        //--cur_indent;
        // emit_indent();
        *m_it++ = '}';
    }

protected:
    It m_it;
    std::string_view indent_str = " ";
    size_t cur_indent = 0;
    bool begun_map_or_seq = false;

    constexpr void emit_indent() {
        /*for (size_t i = 0; i < cur_indent; i++) {
            m_it = std::copy(indent_str.cbegin(), indent_str.cend(), m_it);
        }*/
    }

    constexpr void emit_newline() {
        //*m_it++ = '\n';
    }

    template<typename T>
        requires(std::floating_point<T> || std::integral<T>)
    static constexpr It serialize_primitive(It it, T const& v) {
        std::array<char, 128> buffer;

        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), v);

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

enum class JSONTokenType {
    Number,
    String,
    ArrayBegin,
    ArrayEnd,
    ObjectBegin,
    ObjectEnd,
};

template<typename It>
struct JSONParser {
    constexpr void next() {}
};

template<typename It> struct JSONDeserializer {
    constexpr JSONDeserializer(It it)
        : m_it(it) { }

private:
    It m_it;

    constexpr void skip_ws() {}

    constexpr std::expected<double, std::string_view> scan_number() {
        bool negative = false;
        std::string whole;
        std::string fraction;
        bool negative_exponent = false;
        std::string exponent;

        auto scan_digits = [this](auto& target) {
            while ('0' <= *m_it && *m_it <= '9') {
                *target++ = *m_it++;
            }
        };

        if (*m_it == '-') {
            negative = true;
            m_it++;
        }

        if (*m_it == '0') {
            whole = "0";
        } else {
            scan_digits(back_inserter(whole));
        }

        if (whole.empty()) {
            return std::unexpected { "bad number (whole part)" };
        }

        if (*m_it == '.') {
            m_it++;
            scan_digits(back_inserter(fraction));

            if (fraction.empty()) {
                return std::unexpected { "bad number (fraction part)" };
            }
        }

        if (*m_it == 'e' || *m_it == 'E') {
            if (*m_it == '-' || *m_it == '+') {
                negative_exponent = *m_it == '-';
                m_it++;
            }

            scan_digits(back_inserter(exponent));

            if (fraction.empty()) {
                return std::unexpected { "bad number (exponent part)" };
            }
        }

        return 0.;
    }

    constexpr std::expected<std::string, std::string_view> scan_string() {
        return std::unexpected { "asd" };
    };
};

}

struct JSONSettings {
    bool indent_object = true;
    std::string_view object_indent = " ";
    bool indent_array = true;
    std::string_view array_indent = " ";
};

template<typename It> struct JSONAdapter {
    using serializer_type = Detail::JSONSerializer<It>;
    using deserializer_type = Detail::JSONDeserializer<It>;

    constexpr JSONAdapter(It it)
        : m_it(it) { }

    constexpr serializer_type make_serializer() { return serializer_type(m_it); }
    constexpr deserializer_type make_deserializer() { return deserializer_type(m_it); }

private:
    It m_it;
};

}
