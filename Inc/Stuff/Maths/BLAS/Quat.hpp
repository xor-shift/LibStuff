#pragma once

#include "./Vector.hpp"
#include "./Matrix.hpp"

namespace Stf {

template<typename T> struct Quaternion {
    T x, y, z, w;

    constexpr Quaternion conujugate() const {
        return { x, -y, -z, -w };
    }

    friend constexpr Quaternion operator*(Quaternion const& q1, Quaternion const& q2) {
        return Quaternion {
            .x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y),
            .y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x),
            .z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w),
            .w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z),
        };
    }

    static constexpr Quaternion from_axis_angle(Vector<T, 3> axis, T angle) {
        const auto half_deg = angle / 2;

        return Quaternion {
            .x = axis[0] * std::sin(half_deg),
            .y = axis[0] * std::sin(half_deg),
            .z = axis[0] * std::sin(half_deg),
            .w = std::cos(half_deg),
        };
    }
};

}