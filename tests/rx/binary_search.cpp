#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>

TEST(BinarySearch, ack) {
  uint32_t const serial_number{0xFFFF'FFE7u};
  uint32_t const decoder_id{0u};
  mdu::rx::BinarySearch binary_search;

  // Start new search
  EXPECT_TRUE(binary_search(serial_number, decoder_id, 255u));

  // Check bit0
  EXPECT_TRUE(binary_search(serial_number, decoder_id, 0u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 64u + 0u));
  // If bit0 was set, ignore from now on
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 128u + 0u));

  // Ignore all further requests
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 1u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 64u + 1u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 2u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 64u + 2u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 3u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 64u + 3u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 4u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 64u + 4u));

  // Up until a new search
  EXPECT_TRUE(binary_search(serial_number, decoder_id, 255u));

  // Check bit0
  EXPECT_TRUE(binary_search(serial_number, decoder_id, 0u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 64u + 0u));
  // If bit0 was clear, ignore from now on
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 192u + 0u));

  // Requests aren't ignored now
  EXPECT_TRUE(binary_search(serial_number, decoder_id, 1u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 64u + 1u));
  EXPECT_TRUE(binary_search(serial_number, decoder_id, 2u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 64u + 2u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 3u));
  EXPECT_TRUE(binary_search(serial_number, decoder_id, 64u + 3u));
  EXPECT_FALSE(binary_search(serial_number, decoder_id, 4u));
  EXPECT_TRUE(binary_search(serial_number, decoder_id, 64u + 4u));
}