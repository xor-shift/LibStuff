#include <Stuff/Competition/Comp.hpp>

#include <gtest/gtest.h>

TEST(Comp, Comp) {
    using namespace fmt;
    using namespace Stf::Comp::Aliases;
    using namespace Stf::Comp;

    vec<i32> vec({ 0, 1, 2, 3, 4, 5, 6 });
    auto res = windowed_foreach(3, vec, [i = 0uz](span<const i32> v) mutable {
        return opt_if(v, [](auto const& v) { return v.size() == 3; }).transform([](auto v) {
            return reduce(begin(v), end(v));
        });
    });
    auto view = res //
              | std::views::filter([](auto v) { return v.has_value(); })
              | std::views::transform([](auto v) { return *v; });

    for (auto i : view) {
        print("{}\n", i);
    }
}