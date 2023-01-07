#pragma once

#include <Stuff/Util/Hacks/Try.hpp>

#include <Stuff/Intro/Intro.hpp>

namespace Stf::Serde {

template<
  typename Serializer, typename T, Stf::ListIntrospector Introspector = Stf::introspector_type<std::remove_cvref_t<T>>>
constexpr tl::expected<void, std::string_view> intro_serialize(Serializer& serializer, T&& v) {
    const size_t sz = Introspector::size(v);

    auto list_ser = TRYX(serializer.serialize_list(sz));

    for (size_t i = 0; i < sz; i++) {
        TRYX(list_ser.serialize_element(Introspector::index(v, i)));
    }

    list_ser.end();

    return {};
}

namespace Detail {

template<typename Serializer, typename T, Stf::TupleIntrospector Introspector, size_t N = 0>
constexpr tl::expected<void, std::string_view> tuple_serialize_helper(Serializer& serializer, T&& v) {
    if constexpr (N == Introspector::size()) {
        return {};
    } else {
        if constexpr (Stf::StructIntrospector<Introspector>) {
            const auto name = Introspector::template key_name<N>();
            TRYX(serializer.serialize_element(name, Introspector::template get<N>(std::forward<T const&>(v))));
        } else {
            TRYX(serializer.serialize_element(Introspector::template get<N>(std::forward<T const&>(v))));
        }

        return tuple_serialize_helper<Serializer, T, Introspector, N + 1>(serializer, std::forward<T>(v));
    }
}

}

template<typename Serializer, typename T, Stf::TupleIntrospector Introspector = Stf::introspector_type<std::decay_t<T>>>
requires (!Stf::StructIntrospector<Introspector>)
constexpr tl::expected<void, std::string_view> intro_serialize(Serializer& serializer, T&& v) {
    auto tup_ser = TRYX(serializer.template serialize_tuple<Introspector::size()>());

    TRYX(Detail::tuple_serialize_helper<decltype(tup_ser), T, Introspector>(tup_ser, std::forward<T>(v)));

    tup_ser.end();
    return {};
}

template<typename Serializer, typename T, Stf::StructIntrospector Introspector = Stf::introspector_type<std::decay_t<T>>>
constexpr tl::expected<void, std::string_view> intro_serialize(Serializer& serializer, T&& v) {
    auto struct_ser = TRYX(serializer.serialize_struct(Introspector::size()));

    TRYX(Detail::tuple_serialize_helper<decltype(struct_ser), T, Introspector>(struct_ser, std::forward<T>(v)));

    struct_ser.end();
    return {};
}

}
