#include "point_test.hpp"

using namespace testing;

TEST_F(PointTest, zpp_entry_all) {
  EXPECT_CALL(*this, zppEntry()).Times(Exactly(1));
  verify({{8u - 1u, 0xFEu},
          {105u - 1u, 0xAAu},
          {106u - 1u, 0x55u},
          {105u - 1u, 0x55u},
          {106u - 1u, 0xAAu},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u}});
}

TEST_F(PointTest, zpp_entry_specific_sn) {
  EXPECT_CALL(*this, zppEntry()).Times(Exactly(1));
  verify({{8u - 1u, 0xFEu},
          {105u - 1u, 0xAAu},
          {106u - 1u, 0x55u},
          {105u - 1u, 0x55u},
          {106u - 1u, 0xAAu},
          {105u - 1u, static_cast<uint8_t>(_serial_number >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_serial_number >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_serial_number >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_serial_number >> 0u)}});
}

TEST_F(PointTest, zsu_entry_specific_id) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(1));
  verify({{8u - 1u, 0xFFu},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u}});
}

TEST_F(PointTest, zsu_entry_specific_sn) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(1));
  verify({{8u - 1u, 0xFFu},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, static_cast<uint8_t>(_serial_number >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_serial_number >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_serial_number >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_serial_number >> 0u)}});
}

TEST_F(PointTest, zsu_entry_all) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(1));
  verify({{8u - 1u, 0xFFu},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u}});
}

TEST_F(PointTest, zsu_missing_reset) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(0));
  verify({// {8u - 1u, 0xFFu},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u}});
}

TEST_F(PointTest, zsu_missing_verify) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(0));
  verify({{8u - 1u, 0xFFu},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          // {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u}});
}

TEST_F(PointTest, zsu_wrong_index) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(0));
  verify({{8u - 1u, 0xFFu},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {107u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)}, // wrong
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u}});
}

TEST_F(PointTest, zsu_wrong_decoder_id) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(0));
  verify({{8u - 1u, 0xFFu},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, (42u & 0x0000'FF00u) >> 8u},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u}});
}

TEST_F(PointTest, zsu_entry_after_reset) {
  EXPECT_CALL(*this, zsuEntry()).Times(Exactly(1));
  verify({{8u - 1u, 0xFFu},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {107u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {8u - 1u, 0xFFu},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 24u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 16u)},
          {105u - 1u, static_cast<uint8_t>(_decoder_id >> 8u)},
          {106u - 1u, static_cast<uint8_t>(_decoder_id >> 0u)},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u},
          {105u - 1u, 0x00u},
          {106u - 1u, 0x00u}});
}
