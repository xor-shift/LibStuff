#include <gtest/gtest.h>

#include <Stuff/Refl/ReflNew.hpp>
#include <Stuff/Refl/Serde.hpp>

namespace Foo {

struct Foo {
    MEMREFL_BEGIN(Foo, 2);

    double a = 3.14159;
    char b[24] = {};
    uint32_t invisible = 0xCAFEBABEu;

    MEMREFL_MEMBER(a);
    MEMREFL_MEMBER(b);
};

}

namespace Bar {

struct Foo {
    MEMREFL_BEGIN(Foo, 2);

    std::array<::Foo::Foo, 16> MEMREFL_DECL_MEMBER(a);
    std::optional<::Foo::Foo> MEMREFL_DECL_MEMBER(b);
    uint32_t invisible = 0xDEADBEEFu;
};

}

struct InternallyReflectableStruct {
    MEMREFL_BEGIN(InternallyReflectableStruct, 2);

    int MEMREFL_DECL_MEMBER(a);
    float MEMREFL_DECL_MEMBER(b);
};

namespace SomeNamespace {

struct ExternallyReflectableStruct {
    int a;
    float b;
};

EXTREFL_BEGIN(SomeNamespace::ExternallyReflectableStruct);
EXTREFL_MEMBER(SomeNamespace::ExternallyReflectableStruct, a);
EXTREFL_MEMBER(SomeNamespace::ExternallyReflectableStruct, b);

}

TEST(Serde, Serde) {
    ASSERT_EQ(Stf::serialized_size_v<int>, sizeof(int));
    ASSERT_EQ(Stf::serialized_size_v<double>, sizeof(double));
    ASSERT_EQ((Stf::serialized_size_v<std::array<int, 4>>), sizeof(int) * 4);
    ASSERT_EQ(Stf::serialized_size_v<std::optional<int>>, sizeof(int) + sizeof(bool));
    ASSERT_EQ(Stf::serialized_size_v<Foo::Foo>, 32);
    ASSERT_EQ(Stf::serialized_size_v<Bar::Foo>, Stf::serialized_size_v<Foo::Foo> * 17 + 1);
    ASSERT_EQ(Stf::serialized_size_v<InternallyReflectableStruct>, sizeof(int) + sizeof(float));

    ASSERT_EQ(Stf::Refl::tuple_size_v<SomeNamespace::ExternallyReflectableStruct>, 2);
    ASSERT_TRUE((std::is_same_v<Stf::Refl::tuple_element_t<0, SomeNamespace::ExternallyReflectableStruct>, int>));
    ASSERT_TRUE((std::is_same_v<Stf::Refl::tuple_element_t<1, SomeNamespace::ExternallyReflectableStruct>, float>));

    // ASSERT_EQ(std::tuple_size_v<SomeNamespace::ExternallyReflectableStruct>, 2);
    // ASSERT_TRUE((std::is_same_v<std::tuple_element_t<0, SomeNamespace::ExternallyReflectableStruct>, int>));
    // ASSERT_TRUE((std::is_same_v<std::tuple_element_t<1, SomeNamespace::ExternallyReflectableStruct>, float>));

    InternallyReflectableStruct asd {};
    InternallyReflectableStruct::MemReflHelper<0>::get(asd) = 1;
    Stf::Refl::get<0>(asd) = 2;

    ASSERT_EQ(asd.a, 2);
    ASSERT_EQ(asd.a, Stf::Refl::get<0>(asd));

    InternallyReflectableStruct a {
        .a = 5,
        .b = 3.1415926,
    };

    SomeNamespace::ExternallyReflectableStruct b {
        .a = 5,
        .b = 3.1415926,
    };

    std::array<uint8_t, Stf::serialized_size_v<decltype(a)>> a_ser;
    Stf::serialize(a_ser.begin(), a);

    InternallyReflectableStruct a_deser {};
    const auto it = Stf::deserialize(a_deser, a_ser.cbegin());
    // ASSERT_EQ(res, Stuff::DeserializeResult::Ok);
    ASSERT_EQ(it, a_ser.cend());
    ASSERT_EQ(a_deser.a, a.a);
    ASSERT_EQ(a_deser.b, a.b);

    std::array<uint8_t, Stf::serialized_size_v<decltype(b)>> b_ser;
    Stf::serialize(b_ser.begin(), b);

    ASSERT_EQ(a_ser.size(), b_ser.size());

    bool all_bytes_are_equal = true;
    for (size_t i = 0; i < a_ser.size(); i++)
        all_bytes_are_equal &= a_ser[i] == b_ser[i];

    ASSERT_TRUE(all_bytes_are_equal);
}

struct ComprehensiveBase {
    MEMREFL_BEGIN(ComprehensiveBase, 9)

    int a = 0;
    float b = 0;
    enum class Inline : int { A, B, C, D, E, F, G, H, I, J, K, L } c = Inline::A;

    int d[2] {};
    float e[2] {};
    Inline f[2] { Inline::A, Inline::A };

    std::array<int[2], 2> g {};

    std::pair<float, Inline> h {};
    std::tuple<float, Inline, std::optional<decltype(h)>> i {};

    MEMREFL_MEMBER(a);
    MEMREFL_MEMBER(b);
    MEMREFL_MEMBER(c);
    MEMREFL_MEMBER(d);
    MEMREFL_MEMBER(e);
    MEMREFL_MEMBER(f);
    MEMREFL_MEMBER(g);
    MEMREFL_MEMBER(h);
    MEMREFL_MEMBER(i);
};

struct Comprehensive {
    MEMREFL_BEGIN(Comprehensive, 1)

    std::pair<std::optional<ComprehensiveBase>[2], float> a;

    MEMREFL_MEMBER(a);
};

TEST(Serde, Comprehensive) {
    ASSERT_EQ(Stf::serialized_size_v<int>, sizeof(int));
    ASSERT_EQ(Stf::serialized_size_v<int[2]>, sizeof(int) * 2);
    ASSERT_EQ((Stf::serialized_size_v<std::array<int, 2>>), sizeof(int) * 2);
    ASSERT_EQ((Stf::serialized_size_v<std::pair<int, int>>), sizeof(int) * 2);
    ASSERT_EQ((Stf::serialized_size_v<std::tuple<int, int>>), sizeof(int) * 2);

    ComprehensiveBase base {
        .a = 1,
        .b = 2.3f,
        .c = ComprehensiveBase::Inline::D,

        .d = { 5, 6 },
        .e = { 7.6f, 8.9f },
        .f = { ComprehensiveBase::Inline::J, ComprehensiveBase::Inline::K },

        .g = { { { 12, 13 }, { 14, 15 } } },
        .h = { 16.17f, ComprehensiveBase::Inline::A },
        .i = { 18.19f, ComprehensiveBase::Inline::B, std::nullopt },
    };

    Comprehensive comp {};
    comp.a.first[0] = std::nullopt;
    comp.a.first[1] = base;
    comp.a.second = 20.21f;

    const size_t expected_base_size = sizeof(int) + sizeof(float) + sizeof(int) + sizeof(int) * 2 + sizeof(float) * 2 + sizeof(int) * 2 + sizeof(int[2]) * 2
        + (sizeof(float) + sizeof(int)) + ((sizeof(float) + sizeof(int)) * 2 + 1);

    ASSERT_EQ(Stf::serialized_size_v<ComprehensiveBase>, expected_base_size);
    ASSERT_EQ(Stf::serialized_size_v<Comprehensive>, (expected_base_size + 1) * 2 + sizeof(float));

    const auto serialized = Stf::serialize(comp);

    const auto deserialized = Stf::deserialize<Comprehensive>(serialized.cbegin());

    ASSERT_EQ(deserialized.a.second, comp.a.second);
    ASSERT_FALSE(deserialized.a.first[0].has_value());
    ASSERT_TRUE(deserialized.a.first[1].has_value());

    // auto const& base = *deserialized.a.first[1];
}
