#include <gtest/gtest.h>

#include <Stuff/Maths/Hash/Sha2.hpp>

#include <fmt/core.h>

template<typename Props, typename T> static void test_vec(T&& vectors, bool capital = true) {
    Stf::Hash::SHA2::SHA2State<Props> state {};

    for (auto [test, expected] : vectors) {
        state.update(test);
        auto digest = state.finish();
        auto got = Stf::Hash::format_digest(digest, capital);
        ASSERT_EQ(expected, got);
        state.reset();
    }
}

template<typename Props> static void unified_test(std::span<std::string_view> hashes) {
    std::array<std::string_view, 8> tests { {
      "",                                                                                 //
      "a",                                                                                //
      "abc",                                                                              //
      "message digest",                                                                   //
      "abcdefghijklmnopqrstuvwxyz",                                                       //
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",                         //
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",                   //
      "12345678901234567890123456789012345678901234567890123456789012345678901234567890", //
    } };

    ASSERT_EQ(hashes.size(), tests.size() + 1);

    Stf::Hash::SHA2::SHA2State<Props> state {};

    for (size_t i = 0; i < tests.size(); i++) {
        state.update(tests[i]);

        auto expected = hashes[i];
        auto got = Stf::Hash::format_digest(state.finish(), true);

        ASSERT_EQ(expected, got);

        state.reset();
    }

    for (auto i = 0uz; i < 100'000uz; i++) {
        state.update("aaaaaaaaaa");
    }

    ASSERT_EQ((Stf::Hash::format_digest(state.finish(), true)), hashes.back());
}

TEST(Hash, SHA2_256) {
    std::array<std::string_view, 9> hashes { {
      "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855",
      "CA978112CA1BBDCAFAC231B39A23DC4DA786EFF8147C4E72B9807785AFEE48BB",
      "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD",
      "F7846F55CF23E14EEBEAB5B4E1550CAD5B509E3348FBC4EFA3A1413D393CB650",
      "71C480DF93D6AE2F1EFAD1447C66C9525E316218CF51FC8D9ED832F2DAF18B73",
      "248D6A61D20638B8E5C026930C3E6039A33CE45964FF2167F6ECEDD419DB06C1",
      "DB4BFCBD4DA0CD85A60C3C37D3FBD8805C77F15FC6B1FDFE614EE0A7C8FDB4C0",
      "F371BC4A311F2B009EEF952DD83CA80E2B60026C8E935592D0F9C308453C813E",
      "CDC76E5C9914FB9281A1C7E284D73E67F1809A48A497200E046D39CCC7112CD0",
    } };

    unified_test<Stf::Hash::SHA2::SHA256Properties>(hashes);
}

TEST(Hash, SHA2_384) {
    std::array<std::string_view, 9> hashes { {
      "38B060A751AC96384CD9327EB1B1E36A21FDB71114BE07434C0CC7BF63F6E1DA274EDEBFE76F65FBD51AD2F14898B95B",
      "54A59B9F22B0B80880D8427E548B7C23ABD873486E1F035DCE9CD697E85175033CAA88E6D57BC35EFAE0B5AFD3145F31",
      "CB00753F45A35E8BB5A03D699AC65007272C32AB0EDED1631A8B605A43FF5BED8086072BA1E7CC2358BAECA134C825A7",
      "473ED35167EC1F5D8E550368A3DB39BE54639F828868E9454C239FC8B52E3C61DBD0D8B4DE1390C256DCBB5D5FD99CD5",
      "FEB67349DF3DB6F5924815D6C3DC133F091809213731FE5C7B5F4999E463479FF2877F5F2936FA63BB43784B12F3EBB4",
      "3391FDDDFC8DC7393707A65B1B4709397CF8B1D162AF05ABFE8F450DE5F36BC6B0455A8520BC4E6F5FE95B1FE3C8452B",
      "1761336E3F7CBFE51DEB137F026F89E01A448E3B1FAFA64039C1464EE8732F11A5341A6F41E0C202294736ED64DB1A84",
      "B12932B0627D1C060942F5447764155655BD4DA0C9AFA6DD9B9EF53129AF1B8FB0195996D2DE9CA0DF9D821FFEE67026",
      "9D0E1809716474CB086E834E310A4A1CED149E9C00F248527972CEC5704C2A5B07B8B3DC38ECC4EBAE97DDD87F3D8985",
    } };

    unified_test<Stf::Hash::SHA2::SHA384Properties>(hashes);
}

TEST(Hash, SHA2_512) {
    // clang-format off
    std::array<std::string_view, 9> hashes { {
      "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E",
      "1F40FC92DA241694750979EE6CF582F2D5D7D28E18335DE05ABC54D0560E0F5302860C652BF08D560252AA5E74210546F369FBBBCE8C12CFC7957B2652FE9A75",
      "DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A2192992A274FC1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F",
      "107DBF389D9E9F71A3A95F6C055B9251BC5268C2BE16D6C13492EA45B0199F3309E16455AB1E96118E8A905D5597B72038DDB372A89826046DE66687BB420E7C",
      "4DBFF86CC2CA1BAE1E16468A05CB9881C97F1753BCE3619034898FAA1AABE429955A1BF8EC483D7421FE3C1646613A59ED5441FB0F321389F77F48A879C7B1F1",
      "204A8FC6DDA82F0A0CED7BEB8E08A41657C16EF468B228A8279BE331A703C33596FD15C13B1B07F9AA1D3BEA57789CA031AD85C7A71DD70354EC631238CA3445",
      "1E07BE23C26A86EA37EA810C8EC7809352515A970E9253C26F536CFC7A9996C45C8370583E0A78FA4A90041D71A4CEAB7423F19C71B9D5A3E01249F0BEBD5894",
      "72EC1EF1124A45B047E8B7C75A932195135BB61DE24EC0D1914042246E0AEC3A2354E093D76F3048B456764346900CB130D2A4FD5DD16ABB5E30BCB850DEE843",
      "E718483D0CE769644E2E42C7BC15B4638E1F98B13B2044285632A803AFA973EBDE0FF244877EA60A4CB0432CE577C31BEB009C5C2C49AA2E4EADB217AD8CC09B",
    } };
    // clang-format on

    unified_test<Stf::Hash::SHA2::SHA512Properties>(hashes);
}

TEST(Hash, SHA2_224) {
    std::array<std::pair<std::string_view, std::string_view>, 3> vectors { {
      { "", "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f" },
      { "The quick brown fox jumps over the lazy dog", "730e109bd7a8a32b1cb9d9a09aa2325d2430587ddbc0c38bad911525" },
      { "The quick brown fox jumps over the lazy dog.", "619cba8e8e05826e9b8c519c0a5c68f4fb653e8a3d8aa04bb2c8cd4c" },
    } };

    test_vec<Stf::Hash::SHA2::SHA224Properties>(vectors, false);
}
