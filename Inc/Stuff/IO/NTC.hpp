#pragma once

#include <array>
#include <cmath>
#include <utility>

namespace Stf::NTC {

enum class Nominal {
    R330 = 330,
    R470 = 470,
    R680 = 680,
    R1000 = 1000,
    R1500 = 1500,
    R2000 = 2000,
    R2200 = 2200,
    R2700 = 2700,
    R3300 = 3300,
    R4700 = 4700,
    R5000 = 5000,
    R6800 = 6800,
    R10K = 10000,
    R12K = 12000,
    R15K = 15000,
    R22K = 22000,
    R33K = 33000,
    R47K = 47000,
    R50K = 50000,
    R68K = 68000,
    R100K = 100000,
    R150K = 150000,
    R220K = 220000,
    R330K = 330000,
    R470K = 470000,
};

constexpr int get_characteristic(Nominal resistance) {
    // clang-format off
    switch (resistance) {
        case Nominal::R330: return 3560;
        case Nominal::R470: return 3560;
        case Nominal::R680: return 3560;
        case Nominal::R1000: return 3528;
        case Nominal::R1500: return 3528;
        case Nominal::R2000: return 3528;
        case Nominal::R2200: return 3977;
        case Nominal::R2700: return 3977;
        case Nominal::R3300: return 3977;
        case Nominal::R4700: return 3977;
        case Nominal::R5000: return 3977;
        case Nominal::R6800: return 3977;
        case Nominal::R10K: return 3977;
        case Nominal::R12K: return 3740;
        case Nominal::R15K: return 3740;
        case Nominal::R22K: return 3740;
        case Nominal::R33K: return 4090;
        case Nominal::R47K: return 4090;
        case Nominal::R50K: return 4190;
        case Nominal::R68K: return 4190;
        case Nominal::R100K: return 4190;
        case Nominal::R150K: return 4370;
        case Nominal::R220K: return 4370;
        case Nominal::R330K: return 4570;
        case Nominal::R470K: return 4570;
        default: return 3977;
    }
    // clang-format on
    std::unreachable();
}

constexpr std::pair<std::array<float, 4>, std::array<float, 4>> get_ntc_coefficients(Nominal resistance) {
    const auto coefficient = get_characteristic(resistance);

    // clang-format off

    switch (coefficient) {
    case 2880: return {{-9.094, 2251.74, 229098, -2.744820E+07,}, {3.354016E-03, 3.495020E-04, 2.095959E-06, 4.260615E-07,}};
    case 3990: return {{-10.2296, 2887.62, 132336, -2.502510E+07,}, {3.354016E-03, 3.415560E-04, 4.955455E-06, 4.364236E-07,}};
    case 3041: return {{-11.1334, 3658.73, -102895, 5.166520E+05,}, {3.354016E-03, 3.349290E-04, 3.683843E-06, 7.050455E-07,}};
    case 3136: return {{-12.4493, 4702.74, -402687, 3.196830E+07,}, {3.354016E-03, 3.243880E-04, 2.658012E-06, -2.701560E-07,}};
    case 3390: return {{-12.6814, 4391.97, -232807, 1.509643E+07,}, {3.354016E-03, 2.993410E-04, 2.135133E-06, -5.672000E-09,}};
    //case 3528: return {{-12.0596, 3687.667, -7617.13, -5.914730E+06,}, {3.354016E-03, 2.909670E-04, 1.632136E-06, 7.192200E-08,}};
    //case 3528: return {{-21.0704, 11903.95, -2504699, 2.470338E+08,}, {3.354016E-03, 2.933908E-04, 3.494314E-06, -7.712690E-07,}};
    case 3560: return {{-13.0723, 4190.574, -47158.4, -1.199256E+07,}, {3.354016E-03, 2.884193E-04, 4.118032E-06, 1.786790E-07,}};
    case 3740: return {{-13.8973, 4557.725, -98275, -7.522357E+06,}, {3.354016E-03, 2.744032E-04, 3.666944E-06, 1.375492E-07,}};
    case 3977: return {{-14.6337, 4791.842, -115334, -3.730535E+06,}, {3.354016E-03, 2.569850E-04, 2.620131E-06, 6.383091E-08,}};
    case 4090: return {{-15.5322, 5229.973, -160451, -5.414091E+06,}, {3.354016E-03, 2.519107E-04, 3.510939E-06, 1.105179E-07,}};
    case 4190: return {{-16.0349, 5459.339, -191141, -3.328322E+06,}, {3.354016E-03, 2.460382E-04, 3.405377E-06, 1.034240E-07,}};
    case 4370: return {{-16.8717, 5759.15, -194267, -6.869149E+06,}, {3.354016E-03, 2.367720E-04, 3.585140E-06, 1.255349E-07,}};
    case 4570: return {{-17.6439, 6022.726, -203157, -7.183526E+06,}, {3.354016E-03, 2.264097E-04, 3.278184E-06, 1.097628E-07,}};
    default: return {{1,1,1,1}, {1,1,1,1}};
    }

    // clang-format on
}

constexpr std::pair<float, float> solve_voltage_divider(float r_known, float v_cc, float v_m) {
    /*
+----------+--- Vcc
|          |
--- +      ||| R1
--- -      |||
|          |
|          +--- Vm
|          |
|         ||| R2
|         |||
|          |
+----------+--- Vref = 0V

    Vcc * R2
        Vm = --------
                   R1 + R2

        1 = (Vcc/Vm) * (R2 / (R1 + R2))

              R1 = R2(Vcc-Vm)/Vm
              R2 = R1*Vm / (Vcc - Vm)

        */

    const float r_1_sol = r_known * (v_cc - v_m) / v_m;
    const float r_2_sol = r_known * v_m / (v_cc - v_m);

    return { r_1_sol, r_2_sol };
}

constexpr float calculate_ntc(Nominal r_nom, float r_ntc) {
    const auto [regular_coeff, inverse_coeff] = get_ntc_coefficients(r_nom);
    auto const& [a, b, c, d] = inverse_coeff;

    const float ln_r_t = std::log(r_ntc / static_cast<float>(static_cast<int>(r_nom)));

    const float pow_0 = 1.f;
    const float pow_1 = ln_r_t * pow_0;
    const float pow_2 = ln_r_t * pow_1;
    const float pow_3 = ln_r_t * pow_2;

    const float quotient = a * pow_0 + b * pow_1 + c * pow_2 + d * pow_3;

    return 1.f / quotient;
}

}
