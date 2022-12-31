#include <gtest/gtest.h>

#include <Stuff/Intro/Introspectors/Span.hpp>

#include <Stuff/Intro/Builder.hpp>
#include <Stuff/Intro/Introspector.hpp>
#include <Stuff/Serde/Serde.hpp>
#include <Stuff/Serde/Serializers/JSON.hpp>

#include <fmt/format.h>
#include <tl/expected.hpp>

namespace BuilderCPP {

struct Bar {
    int a = 1;
    char b = '2';
    std::unique_ptr<int> c = nullptr;
    const char* d = "4";
};

inline constexpr auto _libstf_adl_introspector(Bar&&) {
    auto accessor = Stf::Intro::StructBuilder<Bar> {} //
                      .add_simple<&Bar::a, "a">()
                      .add_simple<&Bar::b, "b">()
                      .add_with_transform<&Bar::c, "c">([]<typename T>(T&& v) { return *v; })
                      .add_with_transform<&Bar::d, "c">([]<typename T>(T&& v) { return std::string_view(v); });
    return accessor;
}

struct Baz {
    Bar a {};
    Bar b[1] { {} };
    std::array<Bar, 1> c { { {} } };
    int d = 2;
};

inline constexpr auto _libstf_adl_introspector(Baz&&) {
    auto accessor = Stf::Intro::StructBuilder<Baz> {} //
                      .add_simple<&Baz::a, "a">()
                      .add_simple<&Baz::b, "b">()
                      .add_simple<&Baz::c, "c">()
                      .add_simple<&Baz::d, "c">();
    return accessor;
}

}

static auto get_baz() {
    BuilderCPP::Baz baz {
        .a = {
          .a = 0,
          .b = '1',
          .c = std::make_unique<int>(2),
          .d = "3.1415926",
        },
        .b = {
          {
            .a = 3,
            .b = '4',
            .c = std::make_unique<int>(5),
            .d = "2.718281",
          },
        },
        .c = {{
          {
            .a = 6,
            .b = '7',
            .c = std::make_unique<int>(8),
            .d = "1.618",
          }
        }},
        .d = 9,
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

TEST(Intro, Builder) {
    const std::string_view expected
      = "[[0, \"1\", 2, \"3.1415926\"], [[3, \"4\", 5, \"2.718281\"]], [[6, \"7\", 8, \"1.618\"]], 9]";

    const BuilderCPP::Baz v_0 = get_baz();
    BuilderCPP::Baz v_1 = get_baz();
    BuilderCPP::Baz&& v_2 = get_baz();

    std::vector<std::optional<std::string>> results;

    results.push_back(serialize_to_string(v_0)); //const lvalue
    results.push_back(serialize_to_string(v_1)); //lvalue
    results.push_back(serialize_to_string(static_cast<BuilderCPP::Baz const&&>(v_2))); //const rvalue
    results.push_back(serialize_to_string(std::move(v_2))); //rvalue

    for (auto const& res : results) {
        ASSERT_TRUE(res);
        ASSERT_EQ(*res, expected);
    }
}
