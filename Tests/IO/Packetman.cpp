#include <gtest/gtest.h>

#include <Stuff/IO/Packetman.hpp>

struct Asd {
    NEW_MEMREFL_BEGIN(Asd, 2);

    int a = 5;
    float b = 3.1415926;

    NEW_MEMREFL_MEMBER(a);
    NEW_MEMREFL_MEMBER(b);
};

TEST(PacketMan, PacketMan) {
    //Stuff::PacketManager<1024, 1024> manager([](const uint8_t*, size_t) {});
    //Asd asd {};
    //manager.send_packet(asd);
}