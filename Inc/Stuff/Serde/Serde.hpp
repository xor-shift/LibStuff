#pragma once

#include <optional>
#include <span>
#include <variant>

#include <Stuff/Serde/Refl/Refl.hpp>

#include "./BasicSerializers.hpp"

namespace Stf::Serde::New {

template<typename T, Adapter A>
constexpr void serialize(T const& v, A& adapter) {
    auto serializer = adapter.make_serializer();
    serialize(v, serializer);
}

template<typename T, Adapter A>
constexpr void deserialize_into(T& v, A& adapter) {
    auto deserializer = adapter.make_deserializer();
    deserialize(v, deserializer);
}

}
