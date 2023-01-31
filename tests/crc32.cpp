#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <mdu/mdu.hpp>
#include <string_view>
#include <vector>

namespace {

// This is a slightly modified version of the original fucked up pseudo CRC32
// implementation found in MXULF code. I don't know what the fuck that's
// supposed to be, but it's definitely NOT a CRC. Also the API makes me want to
// kill myself. The original code contained a comment to please call this
// function with additional 4x zeros to (quote-unquote) "match the checksum
// calculated by the fast algorithm". Wanna know what's funny about this? It
// doesn't either fucking way. To whomever wrote this piece of shit algorithm
// I'd strongly suggest another carreer choice... like how about carpenter or
// postman? I don't know, your choice, but please, please whatever you do STAY
// THE FUCK AWAY from coding.
uint32_t slow_crc32(uint32_t sum, unsigned char const* p, uint32_t len) {
  while (len--) {
    int i;
    unsigned char byte = *p++;

    for (i = 0; i < 8; ++i) {
      uint32_t osum = sum;
      sum <<= 1;
      if (byte & 0x80) sum |= 1;
      if (osum & 0x80000000) sum ^= 0x4C11DB7;
      byte <<= 1;
    }
  }
  return sum;
}

constexpr std::array<uint8_t, 4uz> zeros{};

}  // namespace

static_assert(sizeof(mdu::Crc32) == 4uz);

TEST(Crc32, old_fucked_up_pseudo_crc32_matches_new_one) {
  std::string_view str{"Hello World"};
  std::vector<uint8_t> v(size(str));
  std::ranges::copy(str, begin(v));

  uint32_t crc_old{slow_crc32(0xFFFF'FFFFu, &*begin(v), size(v))};
  crc_old = slow_crc32(crc_old, begin(zeros), 4u);

  mdu::Crc32 crc;
  crc.next(v);
  EXPECT_EQ(crc_old, crc.value());
  EXPECT_EQ(crc.value(), 0x29EE'5C18u);
}

TEST(Crc32, crc32) {
  std::array<uint8_t, 11uz> str{
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
  EXPECT_EQ(mdu::crc32(str), 0x29EE'5C18u);
}