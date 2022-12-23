// TODO: add proper platform detection
#ifdef __i386__

#include <Stuff/Util/CPUID/Features.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <mutex>
#include <tuple>

namespace Stf::CPUID {

namespace Detail {

inline constexpr std::pair<std::string_view, Vendor> k_vendor_lookup[] {
    { "AMDisbetter!", Vendor::AMD },
    { "AuthenticAMD", Vendor::AMD },
    { "CentaurHauls", Vendor::Centaur },
    { "CyrixInstead", Vendor::Cyrix },
    { "GenuineIntel", Vendor::Intel },
    { "TransmetaCPU", Vendor::Transmeta },
    { "GenuineTMx86", Vendor::Transmeta },
    { "Geode by NSC", Vendor::NationalSemi },
    { "NexGenDriven", Vendor::NexGen },
    { "RiseRiseRise", Vendor::Rise },
    { "SiS SiS SiS ", Vendor::SiS },
    { "UMC UMC UMC ", Vendor::UMC },
    { "VIA VIA VIA ", Vendor::VIA },
    { "Vortex86 SoC", Vendor::Vortex86 },
    { "  Shanghai  ", Vendor::Zhaoxin },
    { "HygonGenuine", Vendor::Hygon },
    { "E2K MACHINE ", Vendor::Elbrus },
    { "MiSTer AO486", Vendor::AO486 },
    { "bhyve bhyve ", Vendor::Bhyve },
    { " KVMKVMKVM  ", Vendor::KVM },
    { "TCGTCGTCGTCG", Vendor::QEMU },
    { "Microsoft Hv", Vendor::WindowsVPC },
    { "MicrosoftXTA", Vendor::MicrosoftX86ARM },
    { " lrpepyh  vr", Vendor::Parallels },
    { "VMwareVMware", Vendor::VMware },
    { "XenVMMXenVMM", Vendor::XenHVM },
    { "ACRNACRNACRN", Vendor::ProjectACRN },
    { " QNXQVMBSQG ", Vendor::QNX },
    { "VirtualApple", Vendor::Rosetta },
};

}

inline std::array<uint32_t, 4> call_cpuid(uint32_t leaf) {
    std::array<uint32_t, 4> ret {};

    asm("cpuid\n" //
        : "=a"(ret[0]), "=b"(ret[1]), "=c"(ret[2]), "=d"(ret[3])
        : "0"(leaf));

    return ret;
}

static struct CPUIDState {
    std::mutex mutex {};
    std::atomic_bool initialised = false;

    uint32_t highest_func_param = 0;
    std::array<char, 12> vendor_string_arr {};
    Vendor vendor = Vendor::Unknown;

    uint8_t stepping_id;
    uint8_t model;
    uint8_t family_id;
    uint8_t processor_type;
    uint8_t extended_family_id;

    inline std::string_view vendor_string() const { return { vendor_string_arr.data(), vendor_string_arr.size() }; }

    void initialise() {
        std::unique_lock lock { mutex };

        if (initialised)
            return;

        process_leaf(call_cpuid(0), std::integral_constant<size_t, 0> {});
        process_leaf(call_cpuid(1), std::integral_constant<size_t, 1> {});

        initialised = true;
    }

    void init_guard() {
        if (!initialised)
            initialise();
    }

    void process_leaf(std::array<uint32_t, 4> leaf, std::integral_constant<size_t, 0>) {
        highest_func_param = leaf[0];

        for (auto i = 0uz; i < 3; i++) {
            const auto j = 2 - ((i + 2) % 3);
            const auto chars = std::bit_cast<std::array<char, 4>>(leaf[j + 1]);
            std::copy(chars.cbegin(), chars.cend(), vendor_string_arr.data() + i * 4);
        }

        if (const auto* it = std::ranges::find_if(
              Detail::k_vendor_lookup, [this](auto const& v) { return v.first == vendor_string(); }
            );
            it != std::end(Detail::k_vendor_lookup)) {
            vendor = it->second;
        }
    }

    void process_leaf(std::array<uint32_t, 4> leaf, std::integral_constant<size_t, 1>) {
        const auto extract_bits = [&](size_t reg, uint32_t bits) {
            const auto ret = leaf[reg] & ((1 << bits) - 1);
            leaf[reg] >>= bits;
            return ret;
        };

        stepping_id = extract_bits(0, 4);
        model = extract_bits(0, 4);
        family_id = extract_bits(0, 4);
        processor_type = extract_bits(0, 2);
        std::ignore = extract_bits(0, 2);
        extended_family_id = extract_bits(0, 8);
    }

    void process_leaf(std::array<uint32_t, 4> leaf, std::integral_constant<size_t, 2>) { }
} s_cpuid_state {};

// clang-format off
std::string_view vendor_string() noexcept { s_cpuid_state.init_guard(); return s_cpuid_state.vendor_string(); }
Vendor vendor() noexcept { s_cpuid_state.init_guard(); return s_cpuid_state.vendor; }
uint8_t stepping_id() noexcept { s_cpuid_state.init_guard(); return s_cpuid_state.stepping_id; }
uint8_t model() noexcept { s_cpuid_state.init_guard(); return s_cpuid_state.model; }
uint8_t family_id() noexcept { s_cpuid_state.init_guard(); return s_cpuid_state.family_id; }
uint8_t processor_type() noexcept { s_cpuid_state.init_guard(); return s_cpuid_state.processor_type; }
uint8_t extended_family_id() noexcept { s_cpuid_state.init_guard(); return s_cpuid_state.extended_family_id; }
// clang-format on

bool have_feature(Feature feature) noexcept {
    s_cpuid_state.init_guard();
    //
    return false;
}

}

#endif
