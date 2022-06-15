#include "gtest/gtest.h"

#include <fstream>

#include <Stuff/Graphics/Image.hpp>

TEST(Image, Idk) {
    Stf::Gfx::Image image(4, 8, Stf::Gfx::Colors::black);

    const auto asd = image.dimensions();

    const auto [w, h] = image.dimensions();

    ASSERT_EQ(image.size(), 32 * 4);
    ASSERT_EQ(w, 4);
    ASSERT_EQ(h, 8);
}

// Not really a test
TEST(Image, QoI) {
    const char* const in_qoi = "Tests/Graphics/Images/testcard.qoi";
    const char* const out_data = "Tests/Graphics/out.data";
    const char* const reencode_qoi = "Tests/Graphics/reencode.qoi";
    const char* const redecode_data = "Tests/Graphics/redecode.data";

    std::ifstream ifs;
    std::ofstream ofs;

    ifs.open(in_qoi, std::ios::binary);
    ASSERT_TRUE(ifs);
    std::vector<uint8_t> vec_in_qoi { std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
    ifs.close();

    auto res_in_qoi = Stf::Gfx::Formats::QoI::decode(std::span(vec_in_qoi));
    ASSERT_TRUE(res_in_qoi);
    auto& img_in_qoi = *res_in_qoi;

    ofs.open(out_data, std::ios::binary | std::ios::out);
    ASSERT_TRUE(ofs);
    std::ranges::copy(img_in_qoi, std::ostreambuf_iterator<char>(ofs));
    ofs.close();

    std::vector<uint8_t> vec_reencode_qoi{};
    Stf::Gfx::Formats::QoI::encode(back_inserter(vec_reencode_qoi), img_in_qoi);

    ofs.open(reencode_qoi, std::ios::binary | std::ios::out);
    ASSERT_TRUE(ofs);
    std::ranges::copy(vec_reencode_qoi, std::ostreambuf_iterator<char>(ofs));
    ofs.close();

    auto res_redecode_data = Stf::Gfx::Formats::QoI::decode(std::span(vec_reencode_qoi));
    ASSERT_TRUE(res_redecode_data);
    auto& img_redecode_data = *res_redecode_data;

    ofs.open(redecode_data, std::ios::binary | std::ios::out);
    ASSERT_TRUE(ofs);
    std::ranges::copy(img_redecode_data, std::ostreambuf_iterator<char>(ofs));
    ofs.close();
}
