#include <gtest/gtest.h>

#include <Stuff/IO/Delim.hpp>
#include <Stuff/IO/GPS.hpp>

TEST(GPS, GPS) {
    const std::string_view data = "$GPRMC,,V,,,,,,,,,,N*53\r\n"
                                  "$GPVTG,,,,,,,,,N*30\r\n"
                                  "$GPGGA,,,,,,0,00,99.99,,,,,,*40\r\n"
                                  "$GPGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30\r\n"
                                  "$GPGSV,1,1,00*79\r\n"
                                  "$GPGLL,,,,,,V,N*64\r\n"
                                  "$GPRMC,143026.00,A,3703.63512,N,03521.05260,E,0.224,,040322,,,A*71\r\n"
                                  "$GPVTG,,T,,M,0.224,N,0.416,K,A*24\r\n"
                                  "$GPGGA,143026.00,3703.63512,N,03521.05260,E,1,04,1.70,134.6,M,27.5,M,,*5C\r\n"
                                  "$GPGSA,A,3,12,18,05,31,,,,,,,,,11.64,1.70,11.51*0F\r\n"
                                  "$GPGSV,3,1,11,02,29,045,,05,33,107,35,11,25,044,15,12,41,122,48*7E\r\n"
                                  "$GPGSV,3,2,11,18,34,214,32,20,32,074,21,24,00,173,,25,83,102,40*7B\r\n"
                                  "$GPGSV,3,3,11,29,63,321,45,31,32,295,44,39,46,196,42*4B\r\n"
                                  "$GPGLL,3703.63512,N,03521.05260,E,143026.00,A,A*6B\r\n";

    Stf::GPS::GPSState state {};
    Stf::BufferedDelimitedReader<char> reader("\r\n");

    for (const auto c : data) {
        if (!reader.feed_char(c)) {
            auto message = reader.get_buffered_message();

            state.feed_line(message);

            reader.reset();

            ASSERT_TRUE(reader.feed_char(c));
        }
    }

    ASSERT_TRUE(state.time);
    ASSERT_EQ(state.time->hh, 14);
    ASSERT_EQ(state.time->mm, 30);
    ASSERT_EQ(state.time->ss, 26);
    ASSERT_EQ(state.time->sss, 0);

    ASSERT_FALSE(state.date);

    ASSERT_TRUE(state.location);

    ASSERT_EQ(state.location->first.degrees, 37);
    ASSERT_EQ(state.location->first.minutes, 3);
    ASSERT_EQ(state.location->first.minutes_decimal_digits, 5);
    ASSERT_EQ(state.location->first.minutes_decimal, 63512);
    ASSERT_EQ(state.location->first.as_degrees(), 37.060585333333336f);

    ASSERT_EQ(state.location->second.degrees, 35);
    ASSERT_EQ(state.location->second.minutes, 21);
    ASSERT_EQ(state.location->second.minutes_decimal_digits, 5);
    ASSERT_EQ(state.location->second.minutes_decimal, 5260);
    ASSERT_EQ(state.location->second.as_degrees(), 35.350876666666665f);

    //ASSERT_EQ(state.ns_direction, Stf::GPS::Direction::E);
    //ASSERT_EQ(state.ew_direction, Stf::GPS::Direction::N);

    ASSERT_EQ(state.connected_satellites, 4);
}