#include <gtest/gtest.h>

#include <Stuff/Intro/Introspectors/Span.hpp>

#include <Stuff/Intro/Builder.hpp>
#include <Stuff/Intro/Introspector.hpp>
#include <Stuff/Serde/Serde.hpp>
#include <Stuff/Serde/Serializers/JSON.hpp>

#include <fmt/format.h>
#include <tl/expected.hpp>

namespace Foo {

struct Bar {
    int a;
    char b;
    std::string_view c;
};

struct Baz {
    Bar a[1];
    std::array<Bar, 1> b;
    std::unique_ptr<int> c;
    const char* d;
};

inline constexpr auto _libstf_adl_introspector(Baz&&) {
    auto accessor = Stf::Intro::StructBuilder<Baz> {} //
                      .add_simple<&Baz::a, "a">()
                      .add_simple<&Baz::b, "b">()
                      .add_with_transform<&Baz::c, "c">([]<typename T>(T&& v) { return *v; })
                      .add_with_transform<&Baz::d, "d">([]<typename T>(T&& v) { return std::string_view(v); });
    return accessor;
}

}

static auto get_baz() {
    Foo::Baz baz {
        .a = {
          {
            .a = 0,
            .b = '1',
            .c = "3.1415926",
          },
        },
        .b = {{
          {
            .a = 2,
            .b = '3',
            .c = "2.718281",
          }
        }},
        .c = std::make_unique<int>(4),
        .d = "56",
    };

    return baz;
}

template<typename T> static std::optional<std::string> serialize_to_string(T&& v) {
    std::string str {};
    std::ostringstream oss(str);
    Stf::Serde::JSON::Serializer<std::ostringstream> ser { oss };
    if (!Stf::serialize(ser, std::forward<T>(v)).has_value())
        return std::nullopt;
    oss.flush();
    return oss.str();
}

TEST(Serde, JSON) {
    const std::string_view expected
      = "{\"a\":[[0,\"1\",\"3.1415926\"]],\"b\":[[2,\"3\",\"2.718281\"]],\"c\":4,\"d\":\"56\"}";

    const Foo::Baz v_0 = get_baz();
    Foo::Baz v_1 = get_baz();
    Foo::Baz&& v_2 = get_baz();

    std::vector<std::optional<std::string>> results;

    results.push_back(serialize_to_string(v_0)); //const lvalue
    results.push_back(serialize_to_string(v_1)); //lvalue
    results.push_back(serialize_to_string(static_cast<Foo::Baz const&&>(v_2))); //const rvalue
    results.push_back(serialize_to_string(std::move(v_2))); //rvalue

    for (auto const& res : results) {
        ASSERT_TRUE(res);
        ASSERT_EQ(*res, expected);
    }
}
