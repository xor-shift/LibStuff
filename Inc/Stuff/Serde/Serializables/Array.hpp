#pragma once

#include <Stuff/Util/Hacks/Try.hpp>

#include <array>
#include <cstddef>

template<typename Serializer, typename T, size_t N>
constexpr tl::expected<void, std::string_view> _libstf_adl_serializer(Serializer& serializer, const T (&v)[N]) {
    auto tup_ser = TRYX(serializer.template serialize_tuple<N>());

    for (size_t i = 0; i < N; i++) {
        TRYX(tup_ser.serialize_element(v[i]));
    }

    tup_ser.end();

    return {};
}

namespace std {



}
