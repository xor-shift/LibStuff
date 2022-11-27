#include <fmt/format.h>

#include <Stuff/Util/CPUID/Features.hpp>

int main() {
    fmt::print("Architecture:            ???\n");
    fmt::print("  CPU op-mode(s):        ???\n");
    fmt::print("  Address sizes:         ???\n");
    fmt::print("  Byte Order:            ???\n");
    fmt::print("CPU(s):                  ???\n");
    fmt::print("  On-line CPU(s) list:   ???\n");
    fmt::print("Vendor ID:               {}\n", Stf::CPUID::vendor_string());
    fmt::print("  Model name:            ???\n");
    fmt::print("    CPU family:          {}\n", Stf::CPUID::family_id());
    fmt::print("    Model:               {}\n", Stf::CPUID::model());
    fmt::print("    Thread(s) per core:  ???\n");
    fmt::print("    Thread(s) per core:  ???\n");
    fmt::print("    Socket(s):           ???\n");
    fmt::print("    Stepping:            {}\n", Stf::CPUID::stepping_id());
    fmt::print("    Frequency boost:     ???\n");
    fmt::print("    CPU(s) scaling MHz:  ???\n");
    fmt::print("    CPU max MHz:         ???\n");
    fmt::print("    CPU min MHz:         ???\n");
    fmt::print("    BogoMIPS:            ???\n");
}