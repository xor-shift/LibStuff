#include <gtest/gtest.h>

#include <array>

#include <fmt/format.h>

// #include <Stuff/Serde/Serializables/Array.hpp>

#include <Stuff/Intro/Intro.hpp>
#include <Stuff/Intro/Introspectors/Array.hpp>
#include <Stuff/Serde/IntroSerializers.hpp>
#include <Stuff/Serde/Serde.hpp>
#include <Stuff/Serde/Serializers/JSON.hpp>


namespace SerdeCPP {

struct Bar {
    int a = 1;
    char b = '2';
    std::string c = "3";
    std::string_view d = "4";
};

struct Baz {
    Bar a {};
    Bar b[1] { {} };
    std::array<Bar, 1> c { { {} } };
    int d = 2;
};

}

TEST(Serde, JSON) {
    std::string str {};
    std::ostringstream oss(str);
    Stf::Serde::JSON::Serializer<std::ostringstream> ser { oss };
    ASSERT_TRUE(Stf::serialize(ser, SerdeCPP::Baz {}).has_value());
    oss.flush();
    fmt::print("{}\n", oss.str());
}
