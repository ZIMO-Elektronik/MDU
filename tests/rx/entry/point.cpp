#include "point_test.hpp"

using namespace ::testing;

TEST_F(PointTest, zsu_entry_specific_id) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(1));
  verify({{8u - 1u, 255},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, zsu_entry_all) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(1));
  verify({{8u - 1u, 255},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, zsu_missing_reset) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(0));
  verify({// {8u - 1u, 255},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, zsu_missing_verify) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(0));
  verify({{8u - 1u, 255},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          // {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, zsu_wrong_index) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(0));
  verify({{8u - 1u, 255},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {107u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},  // wrong
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, zsu_wrong_decoder_id) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(0));
  verify({{8u - 1u, 255},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, (42u & 0x0000'FF00u) >> 8u},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, zsu_entry_after_reset) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(1));
  verify({{8u - 1u, 255},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {107u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {8u - 1u, 255},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}