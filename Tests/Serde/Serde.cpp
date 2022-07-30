#include <Stuff/Serde/Adapters/BEncode.hpp>
#include <Stuff/Serde/Adapters/JSON.hpp>
#include <Stuff/Serde/Serde.hpp>

#include <fmt/format.h>
#include <gtest/gtest.h>

struct Primitives {
    bool p_b = {};
    uint8_t p_u8 = 1;
    uint16_t p_u16 = 2;
    uint32_t p_u32 = 3;
    uint64_t p_u64 = 4;
    int8_t p_i8 = -5;
    int16_t p_i16 = -6;
    int32_t p_i32 = -7;
    int64_t p_i64 = -8;

    __uint128_t p_u128 = 9;
    __int128_t p_i128 = -10;

    char p_c = 'A';
    wchar_t p_wc = 'B';
    char8_t p_c8 = 'C';
    char16_t p_c16 = 'D';
    char32_t p_c32 = 'E';

    float p_f32 = 1.618f;
    double p_f64 = 2.718;
    long double p_f80 = 3.1415;

    INT_REFL(Primitives,                //
        (&Primitives::p_b, "p_b")       //
        (&Primitives::p_u8, "p_u8")     //
        (&Primitives::p_u16, "p_u16")   //
        (&Primitives::p_u32, "p_u32")   //
        (&Primitives::p_u64, "p_u64")   //
        (&Primitives::p_i8, "p_i8")     //
        (&Primitives::p_i16, "p_i16")   //
        (&Primitives::p_i32, "p_i32")   //
        (&Primitives::p_i64, "p_i64")   //
        (&Primitives::p_u128, "p_u128") //
        (&Primitives::p_i128, "p_i128") //
        (&Primitives::p_c, "p_c")       //
        (&Primitives::p_wc, "p_wc")     //
        (&Primitives::p_c8, "p_c8")     //
        (&Primitives::p_c16, "p_c16")   //
        (&Primitives::p_c32, "p_c32")   //
        (&Primitives::p_f32, "p_f32")   //
        (&Primitives::p_f64, "p_f64")   //
        (&Primitives::p_f80, "p_f80")   //
    );
};

TEST(SerdeNew, PrimitiveSerialization) {
    std::string out;
    Stf::Serde::New::JSONAdapter adapter(back_inserter(out));

    Primitives primitives_test_struct {};

    Stf::Serde::New::serialize(primitives_test_struct, adapter);

    fmt::print("{}\n", out);
}

struct ShallowlyNested {
    int a = 1;
    char b = 'a';
    std::string_view c = "bce";
    std::string d = "def";
    int e[2] = { 2, 3 };
    std::array<int, 2> f = { 4, 5 };
    std::pair<int, int> g = { 6, 7 };
    std::tuple<int, int> h = { 8, 9 };

    INT_REFL(ShallowlyNested,          //
        (&ShallowlyNested::a, "a", "") //
        (&ShallowlyNested::b, "b", "") //
        (&ShallowlyNested::c, "c", "") //
        (&ShallowlyNested::d, "d", "") //
        (&ShallowlyNested::e, "e", "") //
        (&ShallowlyNested::f, "f", "") //
        (&ShallowlyNested::g, "g", "") //
        (&ShallowlyNested::h, "h", "") //
    );
};

TEST(SerdeNew, ShallowlyNestedSerialization) {
    std::string out;
    Stf::Serde::New::JSONAdapter adapter(back_inserter(out));

    ShallowlyNested shallowly_nested_struct {};

    Stf::Serde::New::serialize(shallowly_nested_struct, adapter);

    fmt::print("{}\n", out);
}

struct DeeplyNestedStruct {
    std::array<std::optional<ShallowlyNested>, 2> a = { std::nullopt, ShallowlyNested{} };
    std::pair<std::optional<std::tuple<int, int, char>>, std::optional<std::tuple<int, std::string_view, int>>> b {
        std::nullopt,
        std::tuple { 1, "asd", 2 },
    };
    std::array<std::variant<std::monostate, float, int, char, std::string_view>, 8> c {
        3.1415f,
        123,
        'X',
        "asd",
        1,
        std::monostate{},
        3.4f,
        5.6f,
    };
    Primitives d;

    INT_REFL(DeeplyNestedStruct,          //
        (&DeeplyNestedStruct::a, "a", "") //
        (&DeeplyNestedStruct::b, "b", "") //
        (&DeeplyNestedStruct::c, "c", "") //
        (&DeeplyNestedStruct::d, "d", "") //
    );
};

TEST(SerdeNew, DeeplyNestedSerialization) {
    std::string out;
    Stf::Serde::New::JSONAdapter adapter(back_inserter(out));

    DeeplyNestedStruct deeply_nested_struct {};

    Stf::Serde::New::serialize(deeply_nested_struct, adapter);

    fmt::print("{}\n", out);
}
