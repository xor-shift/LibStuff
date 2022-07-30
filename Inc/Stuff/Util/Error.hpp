#pragma once

#include <source_location>
#include <variant>

namespace Stf {

#define unwrap(to, src_expr)      \
    if (auto res = src_expr; res) \
        to = *res;                \
    else                          \
        for (;;)                  \
            std::abort();

#define unwrap_or_return(to, src_expr, ret_expr) \
    if (auto res = src_expr; res)                \
        to = *res;                               \
    else                                         \
        return std::unexpected { ret_expr };
}

#ifdef GOOGLETEST_INCLUDE_GTEST_GTEST_H_

#    define ASSERT_NO_ERROR(expr)                                                                                \
        if (!static_cast<bool>(expr)) {                                                                                   \
            std::cerr << "ASSERT_NO_ERROR for " << #expr << " failed with error: " << expr.error() << std::endl; \
            ASSERT_TRUE(expr);                                                                                   \
        }

#endif
