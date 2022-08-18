#include "gtest/gtest.h"

#include <Stuff/Util/Scope.hpp>

TEST(Scope, Scope) {
    bool exit_guard_activated = false;
    bool disarmed_exit_guard_activated = false;
    bool fail_guard_activated = false;
    bool disarmed_fail_guard_activated = false;

    {
        auto exit_guard = Stf::ScopeGuard(Stf::GuardType::ScopeExit, [&exit_guard_activated] { exit_guard_activated = true; });
    }

    {
        auto exit_guard = Stf::ScopeGuard(Stf::GuardType::ScopeExit, [&disarmed_exit_guard_activated] { disarmed_exit_guard_activated = true; });

        exit_guard.release();
    }

    try {
        ([&fail_guard_activated] {
            auto fail_guard = Stf::ScopeGuard(Stf::GuardType::ScopeExit, [&fail_guard_activated] { fail_guard_activated = true; });
            throw std::runtime_error("aeiou");
        })();
    } catch (...) { }

    try {
        ([&disarmed_fail_guard_activated] {
            auto fail_guard = Stf::ScopeGuard(Stf::GuardType::ScopeExit, [&disarmed_fail_guard_activated] { disarmed_fail_guard_activated = true; });
            fail_guard.release();
            throw std::runtime_error("aeiou");
        })();
    } catch (...) { }

    ASSERT_TRUE(exit_guard_activated);
    ASSERT_FALSE(disarmed_exit_guard_activated);
    ASSERT_TRUE(fail_guard_activated);
    ASSERT_FALSE(disarmed_fail_guard_activated);
}