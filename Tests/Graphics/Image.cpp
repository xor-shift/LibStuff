#include "gtest/gtest.h"

#include <Stuff/Util/Hacks/Try.hpp>

#include <fstream>

#include <Stuff/Files/Format.hpp>
#include <Stuff/Graphics/Image.hpp>
#include <Stuff/Maths/Check/CRC.hpp>

TEST(Image, Idk) {
    Stf::Gfx::Image image(4, 8, Stf::Gfx::Colors::black);

    const auto asd = image.dimensions();

    const auto [w, h] = image.dimensions();

    ASSERT_EQ(image.size(), 32 * 4);
    ASSERT_EQ(w, 4);
    ASSERT_EQ(h, 8);
}

template<typename Allocator = std::allocator<uint8_t>> constexpr uint32_t image_checksum(Stf::Gfx::Image<Allocator> const& image) {
    Stf::CRCState<Stf::CRCDescriptions::CRC32ISOHDLC, true> state {};

    for (const auto v = std::bit_cast<std::array<uint8_t, sizeof(size_t) * 2>>(image.dimensions()); const auto b : v) {
        state.update(b);
    }

    for (auto b : image) {
        state.update(b);
    }

    return state.finished_value();
}

template<typename IIter, typename Allocator = std::allocator<uint8_t>>
constexpr tl::expected<Stf::Gfx::Image<Allocator>, std::string_view> decode_and_validate(IIter begin, IIter end, uint32_t expected_cksum) {
    auto decoded = TRYX(Stf::Gfx::Formats::QoI::decode(begin, end));

    if (const auto cksum = image_checksum<Allocator>(decoded); cksum != expected_cksum)
        return tl::unexpected { "Invalid image checksum" };

    return decoded;
}

template<typename Allocator = std::allocator<uint8_t>>
static tl::expected<Stf::Gfx::Image<Allocator>, std::string_view> decode_and_validate(const char* file, uint32_t expected_cksum) {
    std::ifstream ifs(file, std::ios::binary);
    if (!ifs)
        return tl::unexpected { "Failed to open file" };
    return decode_and_validate<std::istreambuf_iterator<char>, Allocator>(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(), expected_cksum);
}

struct QoITest {
    uint32_t decoded_cksum;

    const char* in_qoi;
    const char* out_data = nullptr;
    const char* reencode_qoi = nullptr;
    const char* redecode_data = nullptr;

    void run() const {
        auto res_in_qoi = decode_and_validate(in_qoi, decoded_cksum);
        ASSERT_TRUE(res_in_qoi);
        auto& img_in_qoi = *res_in_qoi;

        if (out_data != nullptr) {
            std::ofstream ofs(out_data, std::ios::binary | std::ios::out);
            ASSERT_TRUE(ofs);
            std::ranges::copy(img_in_qoi, std::ostreambuf_iterator<char>(ofs));
        }

        std::vector<uint8_t> vec_reencode_qoi {};
        Stf::Gfx::Formats::QoI::encode(back_inserter(vec_reencode_qoi), img_in_qoi);

        if (reencode_qoi != nullptr) {
            std::ofstream ofs(reencode_qoi, std::ios::binary | std::ios::out);
            ASSERT_TRUE(ofs);
            std::ranges::copy(vec_reencode_qoi, std::ostreambuf_iterator<char>(ofs));
        }

        auto res_redecode_data = decode_and_validate(vec_reencode_qoi.begin(), vec_reencode_qoi.end(), decoded_cksum);
        ASSERT_TRUE(res_redecode_data);
        auto img_redecode_data = std::move(*res_redecode_data);

        if (redecode_data != nullptr) {
            std::ofstream ofs(redecode_data, std::ios::binary | std::ios::out);
            ASSERT_TRUE(ofs);
            std::ranges::copy(img_redecode_data, std::ostreambuf_iterator<char>(ofs));
        }
    }
};

TEST(Image, QoI) {
    Stf::FFormat::asd();

    QoITest {
        .decoded_cksum = 0x5E6D564Eul,
        .in_qoi = "Tests/Graphics/Images/testcard.qoi",
        .out_data = "Tests/Graphics/testcard.data",
        .reencode_qoi = "Tests/Graphics/testcard_re.qoi",
        .redecode_data = "Tests/Graphics/testcard_re.data",
    }.run();

    QoITest {
        .decoded_cksum = 0x76B5C764ul,
        .in_qoi = "Tests/Graphics/Images/testcard_rgba.qoi",
        .out_data = "Tests/Graphics/testcard_rgba.data",
        .reencode_qoi = "Tests/Graphics/testcard_rgba_re.qoi",
        .redecode_data = "Tests/Graphics/testcard_rgba_re.data",
    }.run();

    QoITest {
        .decoded_cksum = 0xA53F2646ul,
        .in_qoi = "Tests/Graphics/Images/dice.qoi",
        .out_data = "Tests/Graphics/dice.data",
        .reencode_qoi = "Tests/Graphics/dice_re.qoi",
        .redecode_data = "Tests/Graphics/dice_re.data",
    }.run();
}
