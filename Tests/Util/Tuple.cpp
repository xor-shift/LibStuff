#include <gtest/gtest.h>

#include <fmt/format.h>

#include <Stuff/Intro/Intro.hpp>

TEST(Tuple, NTuple) {
    ASSERT_TRUE((std::is_same_v<Stf::n_tuple_t<int, 3>, std::tuple<int, int, int>>));
    ASSERT_FALSE((std::is_same_v<Stf::n_tuple_t<int, 3>, std::tuple<int, int>>));
    ASSERT_FALSE((std::is_same_v<Stf::n_tuple_t<int, 4>, std::tuple<int, int, int>>));
    ASSERT_TRUE((std::is_same_v<Stf::n_tuple_t<int, 4>, std::tuple<int, int, int, int>>));
}
