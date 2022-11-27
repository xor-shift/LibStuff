#pragma once

#include <string_view>

namespace Stf::CPUID {

enum class Vendor {
    Unknown,
    AMD,
    Centaur,
    Cyrix,
    Intel,
    Transmeta,
    NationalSemi,
    NexGen,
    Rise,
    SiS,
    UMC,
    VIA,
    Vortex86,
    Zhaoxin,
    Hygon,
    Elbrus,
    AO486,
    Bhyve,
    KVM,
    QEMU,
    WindowsVPC,
    MicrosoftX86ARM,
    Parallels,
    VMware,
    XenHVM,
    ProjectACRN,
    QNX,
    Rosetta,
};

//_register: (0, 1, 2, 3) -> (eax, ebx, ecx, edx)
#define FEATURE(_leaf, _register, _bit) \
    (_bit) + ((_register) << 4) + ((_leaf) << 6)

// ([0-9]{1,2})\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\n
// $2 = FEATURE(0, 3, $1), //$3\n$4 = FEATURE(0, 2, $1), //$5\n

// manually rename exceptions like Hypervisor, remove hyphens, remove reserved instances

//     ([a-z0-9]+) = 
//     \U$1 = 

// source for the descriptions is wikipedia
enum class Feature : uint32_t {
    SSE3 = FEATURE(0, 2, 0), //Prescott New Instructions-SSE3 (PNI)
    PCLMULQDQ = FEATURE(0, 2, 1), //PCLMULQDQ
    DTES64 = FEATURE(0, 2, 2), //64-bit debug store (edx bit 21)
    MONITOR = FEATURE(0, 2, 3), //MONITOR and MWAIT instructions (SSE3)
    DSCPL = FEATURE(0, 2, 4), //CPL qualified debug store
    VMX = FEATURE(0, 2, 5), //Virtual Machine eXtensions
    SMX = FEATURE(0, 2, 6), //Safer Mode Extensions (LaGrande)
    EST = FEATURE(0, 2, 7), //Enhanced SpeedStep
    TM2 = FEATURE(0, 2, 8), //Thermal Monitor 2
    SSSE3 = FEATURE(0, 2, 9), //Supplemental SSE3 instructions
    CnxtID = FEATURE(0, 2, 10), //L1 Context ID
    SDBG = FEATURE(0, 2, 11), //Silicon Debug interface
    FMA = FEATURE(0, 2, 12), //Fused multiply-add (FMA3)
    CX16 = FEATURE(0, 2, 13), //CMPXCHG16B instruction
    XTPR = FEATURE(0, 2, 14), //Can disable sending task priority messages
    PDCM = FEATURE(0, 2, 15), //Perfmon & debug capability
    PCID = FEATURE(0, 2, 17), //Process context identifiers (CR4 bit 17)
    DCA = FEATURE(0, 2, 18), //Direct cache access for DMA writes[11][12]
    SSE41 = FEATURE(0, 2, 19), //SSE4.1 instructions
    SSE42 = FEATURE(0, 2, 20), //SSE4.2 instructions
    X2APIC = FEATURE(0, 2, 21), //x2APIC
    MOVBE = FEATURE(0, 2, 22), //MOVBE instruction (big-endian)
    POPCNT = FEATURE(0, 2, 23), //POPCNT instruction
    TSCDeadline = FEATURE(0, 2, 24), //APIC implements one-shot operation using a TSC deadline value
    AES = FEATURE(0, 2, 25), //AES instruction set
    XSAVE = FEATURE(0, 2, 26), //XSAVE, XRESTOR, XSETBV, XGETBV
    OSXSAVE = FEATURE(0, 2, 27), //XSAVE enabled by OS
    AVX = FEATURE(0, 2, 28), //Advanced Vector Extensions
    F16C = FEATURE(0, 2, 29), //F16C (half-precision) FP feature
    RDRND = FEATURE(0, 2, 30), //RDRAND (on-chip random number generator) feature
    Hypervisor = FEATURE(0, 2, 31), //Hypervisor present (always zero on physical CPUs)[13][14]

    FPU = FEATURE(0, 3, 0), //Onboard x87 FPU
    VME = FEATURE(0, 3, 1), //Virtual 8086 mode extensions (such as VIF, VIP, PIV)
    DE = FEATURE(0, 3, 2), //Debugging extensions (CR4 bit 3)
    PSE = FEATURE(0, 3, 3), //Page Size Extension
    TSC = FEATURE(0, 3, 4), //Time Stamp Counter
    MSR = FEATURE(0, 3, 5), //Model-specific registers
    PAE = FEATURE(0, 3, 6), //Physical Address Extension
    MCE = FEATURE(0, 3, 7), //Machine Check Exception
    CX8 = FEATURE(0, 3, 8), //CMPXCHG8 (compare-and-swap) instruction
    APIC = FEATURE(0, 3, 9), //Onboard Advanced Programmable Interrupt Controller
    SEP = FEATURE(0, 3, 11), //SYSENTER and SYSEXIT instructions
    MTRR = FEATURE(0, 3, 12), //Memory Type Range Registers
    PGE = FEATURE(0, 3, 13), //Page Global Enable bit in CR4
    MCA = FEATURE(0, 3, 14), //Machine check architecture
    CMOV = FEATURE(0, 3, 15), //Conditional move and FCMOV instructions
    PAT = FEATURE(0, 3, 16), //Page Attribute Table
    PSE36 = FEATURE(0, 3, 17), //36-bit page size extension
    PSN = FEATURE(0, 3, 18), //Processor Serial Number
    CLFSH = FEATURE(0, 3, 19), //CLFLUSH instruction (SSE2)
    DS = FEATURE(0, 3, 21), //Debug store: save trace of executed jumps
    ACPI = FEATURE(0, 3, 22), //Onboard thermal control MSRs for ACPI
    MMX = FEATURE(0, 3, 23), //MMX instructions
    FXSR = FEATURE(0, 3, 24), //FXSAVE, FXRESTOR instructions, CR4 bit 9
    SSE = FEATURE(0, 3, 25), //SSE instructions (a.k.a. Katmai New Instructions)
    SSE2 = FEATURE(0, 3, 26), //SSE2 instructions
    SS = FEATURE(0, 3, 27), //CPU cache implements self-snoop
    HTT = FEATURE(0, 3, 28), //Hyper-threading
    TM = FEATURE(0, 3, 29), //Thermal monitor automatically limits temperature
    IA64 = FEATURE(0, 3, 30), //IA64 processor emulating x86
    PBE = FEATURE(0, 3, 31), //Pending Break Enable (PBE# pin) wakeup capability
};

#undef FEATURE

extern void init();

extern std::string_view vendor_string() noexcept;
extern Vendor vendor() noexcept;
extern uint8_t stepping_id() noexcept;
extern uint8_t model() noexcept;
extern uint8_t family_id() noexcept;
extern uint8_t processor_type() noexcept;
extern uint8_t extended_family_id() noexcept;

extern bool have_feature(Feature feature) noexcept;

inline bool hypervised() { return have_feature(Feature::Hypervisor); }

}
