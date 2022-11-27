#include <gtest/gtest.h>

#include <fmt/format.h>

#include <Stuff/Serde/Serializables/Array.hpp>

#include <Stuff/Serde/Serde.hpp>
#include <Stuff/Serde/Serializers/JSON.hpp>

namespace Foo {

struct Bar {
    int a = 1;
    char b = 'b';
    std::string c = "Hello, world!";
    std::string_view d = "Hello, JSON!";
};

struct Baz {
    Bar a {};
    Bar b[2] { {}, {} };
    int c = 2;
};

}

namespace Foo {

template<typename Serializer>
constexpr std::expected<void, std::string_view> _libstf_adl_serializer(Serializer& serializer, Bar const& v) {
    auto tup_ser = TRYX(serializer.template serialize_tuple<4>());

    TRYX(tup_ser.serialize_element(v.a));
    TRYX(tup_ser.serialize_element(v.b));
    TRYX(tup_ser.serialize_element(v.c));
    TRYX(tup_ser.serialize_element(v.d));

    tup_ser.end();

    return {};
}

template<typename Serializer> constexpr std::expected<void, std::string_view> _libstf_adl_serializer(Serializer& serializer, Baz const& v) {
    auto tup_ser = TRYX(serializer.template serialize_tuple<3>());

    TRYX(tup_ser.serialize_element(v.a));
    TRYX(tup_ser.serialize_element(v.b));
    TRYX(tup_ser.serialize_element(v.c));

    tup_ser.end();

    return {};
}

}

TEST(Serde, JSON) {
    std::string str {};
    std::ostringstream oss(str);
    Stf::Serde::JSON::Serializer<std::ostringstream> ser { oss };
    *Stf::serialize(ser, Foo::Baz {});
    oss.flush();
    fmt::print("{}\n", oss.str());
}
