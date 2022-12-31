#pragma once

#include <ostream>

#include <Stuff/Intro/Intro.hpp>

namespace Stf {

template<typename T, typename Stream> inline constexpr Stream& dump_to_stream(Stream& stream, T const& v, size_t depth = 0);

namespace Detail {

template<typename Stream>
constexpr Stream& indent_to_depth(Stream& stream, size_t depth) {
    for (size_t i = 0; i < depth; i++)
        stream << ' ';
    return stream;
}

template<typename T, typename Stream> inline constexpr Stream& dump_list(Stream& stream, T const& v, size_t depth = 0) {
    using Intro = Stf::introspector_type<T>;

    //constexpr size_t max_inline_size = 16;
    const size_t size = Intro::size(v);

    indent_to_depth(stream, depth) << '[';
    for (size_t i = 0; i < size; i++) {
        stream << '\n';

        auto const& curr = v[i];
        std::ignore = dump_to_stream(stream, curr, depth + 1);

        if (i != size - 1)
            stream << ", ";
        if (i == size - 1)
            stream << '\n';

        /*for (size_t j = 0; j < max_inline_size && i < size; i++, j++) {
            auto const& curr = v[i];
        }*/
    }

    return indent_to_depth(stream, depth) << ']';
}

template<typename T, typename Stream> inline constexpr Stream& dump_tuple(Stream& stream, T const& v, size_t depth = 0) {
    using Intro = Stf::introspector_type<T>;

    constexpr size_t size = Intro::size();

    const auto single = [&]<size_t I>(std::integral_constant<size_t, I>) {
        stream << '\n';
        dump_to_stream(stream, Intro::template get<I>(v), depth + 1);

        if constexpr (I != size - 1)
            stream << ", ";
        if constexpr (I == size - 1)
            stream << '\n';
    };

    const auto lambda = [&]<size_t... Is>(std::index_sequence<Is...>) {
        (..., single(std::integral_constant<size_t, Is>{}));
    };

    indent_to_depth(stream, depth) << '(';
    lambda(std::make_index_sequence<Intro::size()> {});
    return indent_to_depth(stream, depth) << ')';
}

template<typename T, typename Stream> inline constexpr Stream& dump_object(Stream& stream, T const& v, size_t depth = 0) {
    using Intro = Stf::introspector_type<T>;

    const size_t size = Intro::size(v);

    indent_to_depth(stream, depth) << '{';

    Intro::iterate(v, [&, i = 0uz](typename Intro::key_type const& key, typename Intro::member_type const& value) mutable {
        indent_to_depth(stream << '\n', depth + 1) << '"' << key << "\":\n";
        dump_to_stream(stream, value, depth + 2);

        if (i != size - 1)
            stream << ", ";
        if (i == size - 1)
            stream << '\n';

        ++i;
    });

    return indent_to_depth(stream, depth) << '}';
}

}

template<typename T, typename Stream> inline constexpr Stream& dump_to_stream(Stream& stream, T const& v, size_t depth) {
    if constexpr (requires { stream << v; }) {
        return Detail::indent_to_depth(stream, depth) << v;
    } else if constexpr (requires { typename Stf::introspector_type<T>; }) {
        using Intro = Stf::introspector_type<T>;

        if constexpr (Stf::ArrayIntrospector<Intro> || Stf::ListIntrospector<Intro>) {
            return Detail::dump_list(stream, v, depth);
        } else if constexpr (Stf::TupleIntrospector<Intro>) {
            return Detail::dump_tuple(stream, v, depth);
        } else if constexpr (Stf::MapIntrospector<Intro>) {
            return Detail::dump_object(stream, v, depth);
        } else {
            return stream << "[Opaque Value (!)]";
        }
    } else {
        return stream << "[Opaque Value]";
    }
}

}
