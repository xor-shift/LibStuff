#pragma once

#include "./BLAS/Vector.hpp"
#include "./BLAS/Matrix.hpp"
#include "./BLAS/MatVec.hpp"

#ifdef LIBSTUFF_FMT

#    include <fmt/format.h>

namespace fmt {

template<::Stf::Concepts::VectorExpression E>
requires(!::Stf::Concepts::MatrixExpression<E>) struct formatter<E> {
    bool use_space = false;
    bool use_comma = true;
    bool use_braces = true;

    constexpr auto parse(auto& context) {
        auto it = context.begin();
        auto end = context.end();

        for (; it != end && *it != '}'; it++) {
            parse_impl(*it);
        }

        return it;
    }

    constexpr auto parse_impl(char c) {
        switch (c) {
        case 'c':
            use_comma ^= true;
            break;
        case 's':
            use_space ^= true;
            break;
        case 'b':
            use_braces ^= true;
            break;
        default:
            break;
        }
    }

    constexpr auto format(E const& expr, auto& context) {
        auto out = context.out();

        impl(expr, out);

        return out;
    }

    constexpr auto impl(E const& expr, auto& out) {
        if (use_braces)
            out = format_to(out, "[");
        for (size_t i = 0; i < E::vector_size - 1; i++) {
            out = format_to(out, "{}", expr[i]);

            if (use_comma)
                out = format_to(out, ",");
            if (use_space)
                out = format_to(out, " ");
        }
        out = format_to(out, "{}", expr[E::vector_size - 1]);
        if (use_braces)
            out = format_to(out, "]");
    }
};

template<::Stf::Concepts::MatrixExpression E>
requires(!::Stf::Concepts::VectorExpression<E>) struct formatter<E> {
    using row_type = ::Stf::Detail::MatrixRowExpression<typename E::value_type, E>;
    using row_formatter_type = formatter<row_type>;

    row_formatter_type row_formatter {};

    bool use_newline = false;

    constexpr auto parse(auto& context) {
        auto it = context.begin();
        auto end = context.end();

        for (; it != end && *it != '}'; it++) {
            parse_impl(*it);
            row_formatter.parse_impl(*it);
        }

        return it;
    }

    constexpr auto parse_impl(char c) {
        switch (c) {
        case 'n':
            use_newline ^= true;
            break;
        default:
            break;
        }
    }

    constexpr auto format(E const& expr, auto& context) {
        auto out = context.out();

        if (row_formatter.use_braces)
            out = format_to(out, "[");
        for (size_t i = 0; i < E::rows - 1; i++) {
            row_formatter.impl(expr[i], out);
            if (row_formatter.use_comma)
                out = format_to(out, ",");
            if (row_formatter.use_space)
                out = format_to(out, " ");
            if (use_newline)
                out = format_to(out, "\n");
        }
        row_formatter.impl(expr[E::rows - 1], out);
        if (row_formatter.use_braces)
            out = format_to(out, "]");

        return out;
    }
};

}

#endif