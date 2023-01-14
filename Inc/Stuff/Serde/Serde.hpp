#pragma once

#include <Stuff/Util/Hacks/Try.hpp>

#include <concepts>
#include <span>
#include <string_view>
#include <type_traits>

#include <Stuff/Serde/IntroSerializers.hpp>

template<typename Serializer>
constexpr tl::expected<void, std::string_view> _libstf_adl_serializer(Serializer& serializer, bool v) {
    return serializer.serialize_bool(v);
}

template<typename Serializer>
constexpr tl::expected<void, std::string_view> _libstf_adl_serializer(Serializer& serializer, char v) {
    return serializer.serialize_char(v);
}

template<typename Serializer, std::integral T>
    requires(!std::is_same_v<T, bool>) && (!std::is_same_v<T, char>)
constexpr tl::expected<void, std::string_view> _libstf_adl_serializer(Serializer& serializer, T v) {
    return serializer.serialize_integral(v);
}

template<typename Serializer, std::floating_point T>
constexpr tl::expected<void, std::string_view> _libstf_adl_serializer(Serializer& serializer, T v) {
    return serializer.serialize_float(v);
}

template<typename Serializer>
constexpr tl::expected<void, std::string_view> _libstf_adl_serializer(Serializer& serializer, std::span<uint8_t> v) {
    return serializer.serialize_bytes(v);
}

template<typename Serializer>
constexpr tl::expected<void, std::string_view>
_libstf_adl_serializer(Serializer& serializer, std::basic_string_view<char> str) {
    return serializer.serialize_str(str);
}

namespace Stf {

namespace Detail {

template<typename Serializer, typename T>
concept ADLSerializable = //
  requires(Serializer& serializer, T const& v) { _libstf_adl_serializer(serializer, v); };

template<typename Serializer, typename T>
concept IntroSerializable = //
  requires(Serializer& serializer, T const& v) { Stf::Serde::intro_serialize(serializer, v); };

}

template<typename Serializer, typename T>
    requires(Detail::ADLSerializable<Serializer, T>)
constexpr auto serialize(Serializer& serializer, T const& v) {
    return _libstf_adl_serializer(serializer, v);
}

template<typename Serializer, typename T>
    requires(!Detail::ADLSerializable<Serializer, T>) && (Detail::IntroSerializable<Serializer, T>)
constexpr auto serialize(Serializer& serializer, T const& v) {
    return Serde::intro_serialize(serializer, v);
}

}
