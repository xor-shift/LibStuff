#include <gtest/gtest.h>

#include <fmt/format.h>

#include <Stuff/Intro/Introspectors/Array.hpp>
#include <Stuff/Intro/Introspectors/Map.hpp>
#include <Stuff/Intro/Introspectors/Tuple.hpp>
#include <Stuff/Intro/Introspectors/UnorderedMap.hpp>
#include <Stuff/Intro/Introspectors/Vector.hpp>

#include <Stuff/Intro/Dump.hpp>

#include <Stuff/Intro/Introspector.hpp>

#include <Stuff/Intro/Intro.hpp>

struct T {
    int a;
    float b;
    double c;
    const char* d;
};

TEST(Intro, SAgg) {
    ASSERT_TRUE((Stf::SAggOfN<T, 0>)); // default init
    ASSERT_TRUE((Stf::SAggOfN<T, 1>));
    ASSERT_TRUE((Stf::SAggOfN<T, 2>));
    ASSERT_TRUE((Stf::SAggOfN<T, 3>));
    ASSERT_TRUE((Stf::SAggOfN<T, 4>));
    ASSERT_FALSE((Stf::SAggOfN<T, 5>));
    ASSERT_FALSE((Stf::SAggOfN<T, 6>));
    ASSERT_FALSE((Stf::SAggOfN<T, 7>));

    ASSERT_EQ(Stf::Detail::sagg_arity_upper_bound<T>(), 8);
    ASSERT_EQ(Stf::sagg_arity<T>, 4);

    using V = Stf::sagg_types<T>;
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<0, V>, int>));
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<1, V>, float>));
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<2, V>, double>));
    ASSERT_TRUE((std::is_same_v<std::tuple_element_t<3, V>, const char*>));
}

TEST(Intro, StdContainers) {
    int a = 1;
    const int b = 2;

    using std_arr_type = std::array<int, 5>;

    std::pair<std::string_view, std_arr_type> map_contents[] = {
        { "aaaa", { 0, 1, 2, 3, 4 } },      //
        { "aaab", { 5, 6, 7, 8, 9 } },      //
        { "aaac", { 10, 11, 12, 13, 14 } }, //
        { "aac", { 15, 16, 17, 18, 19 } },  //
        { "ac", { 20, 21, 22, 23, 24 } },   //
        { "c", { 25, 26, 27, 28, 29 } },
    };

    constexpr size_t map_size = sizeof(map_contents) / sizeof(map_contents[0]);

    std::array<int, 5> std_arr { 0, 1, 2, 3, 4 };
    int arr[5] { 5, 6, 7, 8, 9 };
    std::vector<int> vector { 10, 11, 12, 13, 14, 15 };
    std::map<std::string_view, std_arr_type> map(map_contents, map_contents + map_size);
    std::tuple<int, int&, int const&, int&&> tuple(a, a, b, std::forward<int>(a));
    std::unordered_map<std::string_view, std_arr_type> umap(map_contents, map_contents + map_size);

    using std_arr_introspector = Stf::introspector_type<decltype(std_arr)>;
    using arr_introspector = Stf::introspector_type<decltype(arr)>;
    using vector_introspector = Stf::introspector_type<decltype(vector)>;
    using map_introspector = Stf::introspector_type<decltype(map)>;
    using tuple_introspector = Stf::introspector_type<decltype(tuple)>;
    using umap_introspector = Stf::introspector_type<decltype(umap)>;
    using sagg_introspector = Stf::introspector_type<T>;

    static_assert(Stf::ArrayIntrospector<std_arr_introspector>);
    static_assert(Stf::ListIntrospector<std_arr_introspector>);
    static_assert(Stf::ArrayIntrospector<arr_introspector>);
    static_assert(Stf::ListIntrospector<arr_introspector>);
    static_assert(Stf::ListIntrospector<vector_introspector>);
    static_assert(Stf::TupleIntrospector<tuple_introspector>);
    static_assert(Stf::TupleIntrospector<sagg_introspector>);
    static_assert(Stf::MapIntrospector<map_introspector>);
    static_assert(Stf::MapIntrospector<umap_introspector>);

    T foo { 1, 2.3f, 4.5, "Hello, world!" };

    ASSERT_EQ(foo.a, 1);
    ASSERT_EQ(foo.b, 2.3f);

    Stf::introspector_type<T>::get<0>(foo) = 4;
    Stf::introspector_type<T>::get<1>(foo) = 5.6f;

    ASSERT_EQ(foo.a, 4);
    ASSERT_EQ(foo.b, 5.6f);

    Stf::dump_to_stream(std::cout, foo);
}

TEST(Intro, Dump) {
    struct Foo {
        int a;
        int b;
        int c;
    };

    struct Bar {
        std::array<Foo, 2> a;
        std::vector<Foo> b;
        std::string_view c;
        std::unordered_map<std::string_view, int> d;
    };

    Bar bar { .a = { {
                { 0, 1, 2 },
                { 3, 4, 5 },
              } },
              .b = { {
                { 6, 7, 8 },
                { 9, 10, 11 },
              } },
              .c = "Hello, world!",
              .d = {} };

    bar.d["first"] = 1;
    bar.d["second"] = 2;
    bar.d["third"] = 3;

    Stf::dump_to_stream(std::cout, bar);

    /*std::string oss_str {};
    std::ostringstream oss(oss_str);

    std::ostream& os = oss;
    Stf::dump_to_stream(os, bar);
    os.flush();

    std::string const& oss_str_ref = oss.str();
    oss.str();

    fmt::print("{}", oss_str_ref);*/
}
