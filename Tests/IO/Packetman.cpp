#include <gtest/gtest.h>

#include <Stuff/IO/Packetman.hpp>

struct Asd {
    MEMREFL_BEGIN(Asd, 2);

    int a = 5;
    float b = 3.1415926;

    MEMREFL_MEMBER(a);
    MEMREFL_MEMBER(b);
};

TEST(PacketMan, PacketMan) {
    //Stuff::PacketManager<1024, 1024> manager([](const uint8_t*, size_t) {});
    //Asd asd {};
    //manager.send_packet(asd);
}