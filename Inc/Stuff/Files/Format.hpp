#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <tuple>

namespace Stf::FFormat {

namespace Concepts {

template<typename T>
concept FieldExpression = requires() { typename T::field_expression_tag; };

}

enum class Primitive {
    U8,
    I8,

    U16BE,
    U32BE,
    U64BE,
    U128BE,
    I16BE,
    I32BE,
    I64BE,
    I128BE,

    U16LE,
    U32LE,
    U64LE,
    U128LE,
    I16LE,
    I32LE,
    I64LE,
    I128LE,

    F32BE,
    F64BE,
    F32LE,
    F64LE,
};

template<Primitive> struct primitive_respresentation;

#define PRIMITIVE_REPRESENTATION_FACTORY(_name, _repr, _endianness)                                              \
    template<> struct primitive_respresentation<Primitive::_name> {                                              \
        using type = _repr;                                                                                      \
        inline static constexpr std::endian endianness = std::endian::_endianness;                               \
        inline static constexpr bool reverse_bytes = (sizeof(type) != 1) && (endianness != std::endian::native); \
    }

static_assert(std::numeric_limits<unsigned char>::digits == 8);

PRIMITIVE_REPRESENTATION_FACTORY(U8, uint8_t, native);
PRIMITIVE_REPRESENTATION_FACTORY(I8, int8_t, native);

PRIMITIVE_REPRESENTATION_FACTORY(U16BE, uint16_t, big);
PRIMITIVE_REPRESENTATION_FACTORY(U32BE, uint32_t, big);
PRIMITIVE_REPRESENTATION_FACTORY(U64BE, uint64_t, big);
PRIMITIVE_REPRESENTATION_FACTORY(U128BE, __uint128_t, big);
PRIMITIVE_REPRESENTATION_FACTORY(I16BE, uint16_t, big);
PRIMITIVE_REPRESENTATION_FACTORY(I32BE, uint32_t, big);
PRIMITIVE_REPRESENTATION_FACTORY(I64BE, uint64_t, big);
PRIMITIVE_REPRESENTATION_FACTORY(I128BE, __uint128_t, big);

PRIMITIVE_REPRESENTATION_FACTORY(U16LE, int16_t, little);
PRIMITIVE_REPRESENTATION_FACTORY(U32LE, int32_t, little);
PRIMITIVE_REPRESENTATION_FACTORY(U64LE, int64_t, little);
PRIMITIVE_REPRESENTATION_FACTORY(U128LE, __int128_t, little);
PRIMITIVE_REPRESENTATION_FACTORY(I16LE, int16_t, little);
PRIMITIVE_REPRESENTATION_FACTORY(I32LE, int32_t, little);
PRIMITIVE_REPRESENTATION_FACTORY(I64LE, int64_t, little);
PRIMITIVE_REPRESENTATION_FACTORY(I128LE, __int128_t, little);

static_assert(std::numeric_limits<float>::radix == 2);
static_assert(std::numeric_limits<float>::max_exponent == 128);
static_assert(std::numeric_limits<float>::digits == 24);
static_assert(std::numeric_limits<double>::radix == 2);
static_assert(std::numeric_limits<double>::max_exponent == 1024);
static_assert(std::numeric_limits<double>::digits == 53);

PRIMITIVE_REPRESENTATION_FACTORY(F32BE, float, big);
PRIMITIVE_REPRESENTATION_FACTORY(F64BE, double, big);
PRIMITIVE_REPRESENTATION_FACTORY(F32LE, float, little);
PRIMITIVE_REPRESENTATION_FACTORY(F64LE, double, little);

#undef PRIMITIVE_REPRESENTATION_FACTORY

template<Primitive P> using primitive_respresentation_t = typename primitive_respresentation<P>::type;

namespace Detail {

template<typename E> struct FieldExpression { using field_expression_tag = void; };

template<typename E> struct NamedField : public FieldExpression<NamedField<E>> {
    using field_expression_tag = void;
    using representation_type = typename E::representation_type;
    inline static constexpr size_t encoded_size = E::encoded_size;

    E field_expression;
    const char* name;

    E& inner() & { return field_expression; }
    E const& inner() const& { return field_expression; }
    E&& inner() && { return std::move(field_expression); }
    E const&& inner() const&& { return std::move(field_expression); }

    template<std::input_iterator IIter> constexpr auto decode(IIter begin, IIter end, representation_type& out) const {
        return field_expression.decode(begin, end, out);
    }
};

template<Primitive Type> struct PrimitiveField : public FieldExpression<PrimitiveField<Type>> {
    using field_expression_tag = void;
    using representation_type = primitive_respresentation_t<Type>;
    inline static constexpr size_t encoded_size = sizeof(representation_type);

    template<std::input_iterator IIter> constexpr std::optional<IIter> decode(IIter begin, IIter end, representation_type& out) const {
        using U = primitive_respresentation<Type>;

        if (begin == end)
            return std::nullopt;

        std::array<char, encoded_size> arr;

        auto it = begin;
        for (size_t i = 0; i < encoded_size; i++) {
            arr[i] = *it++;

            if (it == end)
                return std::nullopt;
        }

        if (U::reverse_bytes)
            std::reverse(arr.begin(), arr.end());

        out = std::bit_cast<representation_type>(arr);
        return it;
    }
};

template<Concepts::FieldExpression E, size_t Len> struct FieldArrayField : public FieldExpression<FieldArrayField<E, Len>> {
    using field_expression_tag = void;
    using representation_type = std::array<typename E::representation_type, Len>;
    inline static constexpr size_t encoded_size = E::encoded_size * Len;

    E expression;

    template<std::input_iterator IIter> constexpr std::optional<IIter> decode(IIter begin, IIter end, representation_type& out) const {
        auto it = begin;
        for (size_t i = 0; i < Len; i++)
            if (auto res = expression.decode(it, end, out[i]); !res)
                return std::nullopt;
            else
                it = *res;

        return it;
    }
};

template<typename... Es> struct GroupExpression : public FieldExpression<GroupExpression<Es...>> {
    using field_expression_tag = void;
    using representation_type = std::tuple<typename Es::representation_type...>;
    inline static constexpr size_t encoded_size = ((1 + Es::encoded_size), ...);

    std::tuple<Es...> expressions;

    template<std::input_iterator IIter> constexpr std::optional<IIter> decode(IIter begin, IIter end, representation_type& out) const {
        return decode_impl(begin, end, out);
    }

private:
    template<std::input_iterator IIter, size_t N = 0> constexpr std::optional<IIter> decode_impl(IIter it, IIter end, representation_type& out) const {
        auto const& cur_expr = std::get<N>(expressions);
        auto& cur_out = std::get<N>(out);

        auto res = cur_expr.decode(it, end, cur_out);

        if (!res)
            return std::nullopt;

        if constexpr (N + 1 < std::tuple_size_v<decltype(expressions)>)
            return decode_impl<IIter, N + 1>(*res, end, out);
        else
            return *res;
    }
};

}

template<Primitive Type> constexpr auto primitive_field() { return Detail::PrimitiveField<Type> {}; }

template<size_t N, Concepts::FieldExpression E> constexpr auto make_array_field(E expr) { return Detail::FieldArrayField<E, N> { {}, expr }; }

template<typename E> constexpr auto name_field(E&& expr, const char* name) { return Detail::NamedField<E> { {}, expr, name }; }

constexpr auto group_field() { return Detail::GroupExpression<> {}; }

template<Concepts::FieldExpression E1, Concepts::FieldExpression... Es> constexpr auto operator<<(Detail::GroupExpression<Es...> e_0, E1 e_1) {
    return Detail::GroupExpression<Es..., E1> { {}, std::tuple_cat(std::move(e_0.expressions), std::tuple(e_1)) };
}

inline void asd() {
    // clang-format off
    uint8_t data[] = {
        0, 0, 0, 0,
        0, 16,
        0x12, 0x34,
        0, 0, 0, 5,
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16,
    };
    // clang-format on

    auto g_0 = name_field(group_field() << name_field(primitive_field<Primitive::U32BE>(), "crc") << name_field(primitive_field<Primitive::U16BE>(), "length")
                                        << name_field(primitive_field<Primitive::U16BE>(), "id") << name_field(primitive_field<Primitive::U32BE>(), "order"),
        "group");

    static_assert(std::tuple_size_v<decltype(g_0.inner().expressions)> == 4);
    static_assert(std::is_same_v<typename decltype(g_0)::representation_type, std::tuple<uint32_t, uint16_t, uint16_t, uint32_t>>);

    typename decltype(g_0)::representation_type out;
    g_0.decode(data, data + sizeof(data), out);

    std::ignore = out;
}
}
