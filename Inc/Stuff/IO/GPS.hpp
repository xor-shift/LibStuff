#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstring>
#include <numbers>
#include <optional>
#include <string_view>
#include <tuple>

#include <Stuff/Maths/Scalar.hpp>
#include <Stuff/Util/Conv.hpp>

namespace Stf::GPS {

struct Time {
    int hh;
    int mm;
    int ss;
    int sss;
};

struct Date {
    int dd;
    int mm;
    int yy;
};

struct Angle {
    int degrees;
    int minutes;
    int minutes_decimal_digits;
    int minutes_decimal;

    constexpr float as_degrees() const {
        float decimal_mult;
        if (constexpr auto pow_4 = Stf::pow(10.f, -4); minutes_decimal_digits == 4) [[likely]]
            decimal_mult = pow_4;
        else if (constexpr auto pow_5 = Stf::pow(10.f, -5); minutes_decimal_digits == 5) [[likely]]
            decimal_mult = pow_5;
        else [[unlikely]]
            decimal_mult = std::pow(10.f, -minutes_decimal_digits);

        return degrees + ((minutes + minutes_decimal * decimal_mult) / 60.f);
    }

    template<typename T = float> constexpr T as_radians() const {
        return as_degrees() / 180.f * std::numbers::pi_v<float>;
    }

    static float distance(Angle lat_0, Angle long_0, Angle lat_1, Angle long_1);
};

enum class Direction : int {
    N = 0,
    S = 1,
    E = 2,
    W = 3,
};

struct GPSState {
    std::optional<Time> time;

    std::optional<Date> date;

    // latitude, longitude
    std::optional<std::pair<Angle, Angle>> location;

    // ns, ew
    std::optional<std::pair<Direction, Direction>> direction;

    size_t connected_satellites;

    void feed_line(std::string_view line);

private:
    struct UpdateData {
        std::string_view time {};
        std::string_view date {};
        std::string_view latitude {};
        std::string_view longitude {};
        std::string_view ns_direction {};
        std::string_view ew_direction {};
        std::string_view connected_satellites {};
    };

    void update_things(UpdateData const& data);
};

}

#ifdef LIBSTUFF_FMT
#    include <fmt/format.h>

namespace fmt {

template<> struct formatter<::Stf::GPS::Time, char, void> {
    constexpr auto parse(auto& context) { return context.begin(); }

    constexpr auto format(::Stf::GPS::Time const& time, auto& context) {
        return format_to(context.out(), "{:02}:{:02}:{:02}.{:03}", time.hh, time.mm, time.ss, time.sss);
    }
};

template<> struct formatter<::Stf::GPS::Angle, char, void> {
    constexpr auto parse(auto& context) { return context.begin(); }

    constexpr auto format(::Stf::GPS::Angle const& angle, auto& context) {
        return format_to(context.out(), "{}", angle.as_degrees());
    }
};

}

#endif
