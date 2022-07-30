#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include <Stuff/Maths/Scalar.hpp>
#include <Stuff/Util/Conv.hpp>

namespace Stf::Nextion {

extern void nextion_transmit_bytes(std::span<const uint8_t> data);

struct ObjectExpression;

template<typename Object> struct FieldExpression;

template<typename LHS, typename T> struct AssignmentExpression;

struct ObjectExpression {
    std::string_view name;

    constexpr FieldExpression<ObjectExpression> txt() const;

    constexpr FieldExpression<ObjectExpression> val() const;

    constexpr FieldExpression<ObjectExpression> operator[](const char* s) const;

    constexpr bool format(std::span<char>& buf) const {
        if (buf.size() < name.size() + 2)
            return false;

        auto it = buf.begin();
        //*it++ = '"';
        it = std::copy(name.cbegin(), name.cend(), it);
        //*it++ = '"';

        buf = { it, buf.end() };

        return true;
    }
};

template<typename Object> struct FieldExpression {
    Object base;
    std::string_view name;

    constexpr size_t size() const { return base.size() + name.size() + 1; }

    template<typename T> constexpr AssignmentExpression<FieldExpression<Object>, T> operator=(T const& other);

    constexpr bool format(std::span<char>& buf) const {
        if (!base.format(buf))
            return false;

        if (buf.size() < 1 + name.size())
            return false;

        buf[0] = '.';
        const auto end = std::copy(name.cbegin(), name.cend(), buf.begin() + 1);

        buf = { end, buf.end() };

        return true;
    }
};

template<typename LHS, typename T> struct AssignmentExpression;

template<typename LHS, std::integral T> struct AssignmentExpression<LHS, T> {
    LHS base;
    T v;

    constexpr bool format(std::span<char>& buf) const {
        if (!base.format(buf))
            return false;

        if (buf.empty())
            return false;

        buf[0] = '=';
        buf = { buf.begin() + 1, buf.end() };

        return format_impl(buf);
    }

    constexpr bool format_impl(std::span<char>& buf) const {
        const auto is_neg = v < 0;

        const auto abs_v = static_cast<std::make_unsigned_t<T>>(Stf::abs(v));
        const auto short_v = static_cast<uint16_t>(abs_v & 0xFFFF);

        char out_buf[6] = {
            '0',
            'x',
            Conv::to_hex_digit(short_v >> 12),
            Conv::to_hex_digit(short_v >> 8),
            Conv::to_hex_digit(short_v >> 4),
            Conv::to_hex_digit(short_v >> 0),
        };

        if (buf.size() < sizeof(out_buf))
            return false;

        const auto end = std::copy(out_buf, out_buf + sizeof(out_buf), buf.begin());

        buf = { end, buf.end() };

        return true;
    }
};

template<typename LHS> struct AssignmentExpression<LHS, std::string_view> {
    LHS base;
    std::string_view v;

    constexpr bool format(std::span<char>& buf) const {
        if (!base.format(buf))
            return false;

        if (buf.empty())
            return false;

        buf[0] = '=';
        buf = { buf.begin() + 1, buf.end() };

        return format_impl(buf);
    }

    constexpr bool format_impl(std::span<char>& buf) const {
        auto it = buf.begin();
        *it++ = '"';
        it = std::copy(v.begin(), v.end(), it);
        *it++ = '"';
        buf = { it, buf.end() };
        return true;
    }
};

constexpr FieldExpression<ObjectExpression> ObjectExpression::txt() const { return (*this)["txt"]; }
constexpr FieldExpression<ObjectExpression> ObjectExpression::val() const { return (*this)["val"]; }
constexpr FieldExpression<ObjectExpression> ObjectExpression::operator[](const char* s) const { return { *this, s }; }

template<typename Object> template<typename T> constexpr AssignmentExpression<FieldExpression<Object>, T> FieldExpression<Object>::operator=(const T& other) {
    return { *this, other };
}

namespace Literals {

constexpr ObjectExpression operator""_obj(const char* s, size_t n) { return { { s, n } }; }

}

template<typename T> constexpr bool set(std::string_view name, T val) {
    char buf[64];
    std::span<char> sp { buf, buf + sizeof(buf) };

    using U = std::decay_t<T>;
    if constexpr (std::is_same_v<U, char*> || std::is_same_v<U, std::string_view> || std::is_same_v<U, std::string>) {
        const auto expr = ObjectExpression { name }["txt"] = val;
        if (!expr.format(sp))
            return false;
    } else {
        const auto expr = ObjectExpression { name }["val"] = val;
        if (!expr.format(sp))
            return false;
    }

    if (sp.size() < 3)
        return false;

    std::fill(sp.begin(), sp.begin() + 3, '\xff');

    const auto leftover = std::distance(sp.begin() + 3, sp.end());
    const auto length = sizeof(buf) - leftover;

    nextion_transmit_bytes({ reinterpret_cast<const uint8_t*>(buf), length });

    return true;
}

template<std::floating_point T> bool set(std::string_view name, T val, unsigned digits) {
    return set(name, static_cast<long>(std::round(val * std::pow(10, digits))));
}

}
