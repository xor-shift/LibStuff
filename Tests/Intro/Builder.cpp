#include <gtest/gtest.h>

#include <Stuff/Intro/Introspectors/Span.hpp>

#include <Stuff/Intro/Builder.hpp>
#include <Stuff/Intro/Introspector.hpp>
#include <Stuff/Serde/Serde.hpp>
#include <Stuff/Serde/Serializers/JSON.hpp>

#include <tl/expected.hpp>
#include <fmt/format.h>

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
                      .add_with_transform<&Bar::c, "c">([]<typename T>(T&& v) {
                          return *v;
                      })
                      .add_with_transform<&Bar::d, "c">([]<typename T>(T&& v) {
                          return std::string_view(v);
                      });
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

TEST(Intro, Builder) {
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

    std::string str {};
    std::ostringstream oss(str);
    Stf::Serde::JSON::Serializer<std::ostringstream> ser { oss };
    ASSERT_TRUE(Stf::serialize(ser, baz).has_value());
    oss.flush();
    fmt::print("{}\n", oss.str());
}
