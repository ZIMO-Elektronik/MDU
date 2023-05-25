#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <mdu/mdu.hpp>
#include <string_view>

TEST(Crc8, crc8_matches_maxim_dallas_polynomial_0x31) {
  mdu::Crc8 crc;
  std::string_view str{"Hello World"};
  std::ranges::for_each(str,
                        [&crc](auto c) { crc.next(static_cast<uint8_t>(c)); });
  EXPECT_EQ(crc.value(), 26u);
}

TEST(Crc8, crc8) {
  std::array<uint8_t, 11uz> str{
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
  EXPECT_EQ(mdu::crc8(str), 26u);
}