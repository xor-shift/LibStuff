#pragma once

#include <concepts>

namespace Stf::Serde::New {

template<typename T>
concept Adapter = requires(T& v) {
                      typename T::serializer_type;
                      typename T::deserializer_type;
                      { v.make_serializer() } -> std::same_as<typename T::serializer_type>;
                      { v.make_deserializer() } -> std::same_as<typename T::deserializer_type>;
                  };

template<typename T>
concept Serializer = requires(T& v) {
                         typename T::ser_sequence_type;
                         typename T::ser_tuple_type;
                         typename T::ser_map_type;
                         typename T::ser_struct_type;
                     };

template<typename T, typename S>
concept Serializable = requires(T const& v, S& s) {
                           { serialize(v, s) };
                       };

}