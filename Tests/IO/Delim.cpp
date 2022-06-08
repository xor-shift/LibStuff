#include <gtest/gtest.h>

#include <vector>

#include <Stuff/IO/Delim.hpp>

TEST(Delim, COBSWikipedia) {
    std::vector<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> tests = {
        { { 0x00 }, { 0x01, 0x01, 0x00 } }, { { 0x00, 0x00 }, { 0x01, 0x01, 0x01, 0x00 } },
        { { 0x00, 0x11, 0x00 }, { 0x01, 0x02, 0x11, 0x01, 0x00 } },
        { { 0x11, 0x22, 0x00, 0x33 }, { 0x03, 0x11, 0x22, 0x02, 0x33, 0x00 } },
        { { 0x11, 0x22, 0x33, 0x44 }, { 0x05, 0x11, 0x22, 0x33, 0x44, 0x00 } },
        { { 0x11, 0x00, 0x00, 0x00 }, { 0x02, 0x11, 0x01, 0x01, 0x01, 0x00 } }, //
        { {}, {} }, // 6  [01 02 03 ... FD FE]    -> [FF 01 02 03 ... FD FE 00]
        { {}, {} }, // 7  [00 01 02 ... FC FD FE] -> [01 FF 01 02 ... FC FD FE 00]
        { {}, {} }, // 8  [01 02 03 ... FD FE FF] -> [FF 01 02 03 ... FD FE 02 FF 00]
        { {}, {} }, // 9  [02 03 04 ... FE FF 00] -> [FF 02 03 04 ... FE FF 01 01 00]
        //{ {}, {} }, // 10 [03 04 05 ... FF 00 01] -> [FE 03 04 05 ... FF 02 01 00]
    };

    tests[6].second.push_back(0xFF);
    tests[7].first.push_back(0x00);
    tests[7].second.push_back(0x01);
    tests[7].second.push_back(0xFF);
    tests[8].second.push_back(0xFF);
    tests[9].second.push_back(0xFF);
    for (size_t i = 1; i <= 254; i++) {
        tests[6].first.push_back(i);
        tests[6].second.push_back(i);

        tests[7].first.push_back(i);
        tests[7].second.push_back(i);

        tests[8].first.push_back(i);
        tests[8].second.push_back(i);

        tests[9].first.push_back(i + 1);
        tests[9].second.push_back(i + 1);
    };
    tests[6].second.push_back(0x00);
    tests[7].second.push_back(0x00);
    tests[8].first.push_back(0xFF);
    tests[8].second.push_back(0x02);
    tests[8].second.push_back(0xFF);
    tests[8].second.push_back(0x00);
    tests[9].first.push_back(0x00);
    tests[9].second.push_back(0x01);
    tests[9].second.push_back(0x01);
    tests[9].second.push_back(0x00);

    for (auto const& [to_encode, expected_encoding] : tests) {
        std::vector<uint8_t> encoded {};
        encoded.reserve(expected_encoding.size());

        std::vector<uint8_t> encode_scratchpad = to_encode;
        Stf::cobs_encode(
            encode_scratchpad.data(), encode_scratchpad.size(), [&](uint8_t b) { encoded.push_back(b); },
            [&](uint8_t const* buf, size_t n) {
                encoded.reserve(encoded.size() + n);
                for (size_t i = 0; i < n; i++)
                    encoded.push_back(buf[i]);
            });

        ASSERT_EQ(expected_encoding.size(), encoded.size());

        bool all_elems_equal = true;
        for (size_t i = 0; i < expected_encoding.size(); i++) {
            bool cur_ok = expected_encoding[i] == encoded[i];
            all_elems_equal &= cur_ok;
        }

        ASSERT_TRUE(all_elems_equal);

        auto [decode_size, decode_result] = Stf::cobs_decode_inplace(encoded.data(), encoded.size());

        ASSERT_EQ(decode_result, Stf::COBSResult::Ok);
        ASSERT_EQ(decode_size, to_encode.size());

        all_elems_equal = true;
        for (size_t i = 0; i < to_encode.size(); i++) {
            bool cur_ok = to_encode[i] == encoded[i];
            all_elems_equal &= cur_ok;
        }

        ASSERT_TRUE(all_elems_equal);
    }
}

TEST(Delim, COBSFuzz) { }
