#pragma once

// macro from p0779r0
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0779r0.pdf

#define TRYX(m)                                     \
    ({                                              \
        auto res = (m);                             \
        if (!res.has_value())                       \
            return std::unexpected { res.error() }; \
        res.value();                                \
    })
