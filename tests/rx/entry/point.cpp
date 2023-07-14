#include "point_test.hpp"

using namespace ::testing;

TEST_F(PointTest, firmware_entry) {
  EXPECT_CALL(*this, firmwareEntry()).Times(Exactly(1));
  verify({{8u - 1u, 255},
          {105u - 1u, (_decoder_id & 0xFF00'0000u) >> 24u},
          {106u - 1u, (_decoder_id & 0x00FF'0000u) >> 16u},
          {105u - 1u, (_decoder_id & 0x0000'FF00u) >> 8u},
          {106u - 1u, (_decoder_id & 0x0000'00FFu) >> 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, firmware_missing_reset) {
  EXPECT_CALL(*this, firmwareEntry()).Times(Exactly(0));
  verify({// {8u - 1u, 255},
          {105u - 1u, (_decoder_id & 0xFF00'0000u) >> 24u},
          {106u - 1u, (_decoder_id & 0x00FF'0000u) >> 16u},
          {105u - 1u, (_decoder_id & 0x0000'FF00u) >> 8u},
          {106u - 1u, (_decoder_id & 0x0000'00FFu) >> 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, firmware_missing_verify) {
  EXPECT_CALL(*this, firmwareEntry()).Times(Exactly(0));
  verify({{8u - 1u, 255},
          {105u - 1u, (_decoder_id & 0xFF00'0000u) >> 24u},
          // {106u - 1u, (_decoder_id & 0x00FF'0000u) >> 16u},
          {105u - 1u, (_decoder_id & 0x0000'FF00u) >> 8u},
          {106u - 1u, (_decoder_id & 0x0000'00FFu) >> 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, firmware_wrong_index) {
  EXPECT_CALL(*this, firmwareEntry()).Times(Exactly(0));
  verify({{8u - 1u, 255},
          {105u - 1u, (_decoder_id & 0xFF00'0000u) >> 24u},
          {107u - 1u, (_decoder_id & 0x00FF'0000u) >> 16u},
          {105u - 1u, (_decoder_id & 0x0000'FF00u) >> 8u},
          {106u - 1u, (_decoder_id & 0x0000'00FFu) >> 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, firmware_wrong_decoder_id) {
  EXPECT_CALL(*this, firmwareEntry()).Times(Exactly(0));
  verify({{8u - 1u, 255},
          {105u - 1u, (_decoder_id & 0xFF00'0000u) >> 24u},
          {107u - 1u, (_decoder_id & 0x00FF'0000u) >> 16u},
          {105u - 1u, (42u & 0x0000'FF00u) >> 8u},
          {106u - 1u, (_decoder_id & 0x0000'00FFu) >> 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}

TEST_F(PointTest, firmware_entry_after_reset) {
  EXPECT_CALL(*this, firmwareEntry()).Times(Exactly(1));
  verify({{8u - 1u, 255},
          {105u - 1u, (_decoder_id & 0xFF00'0000u) >> 24u},
          {107u - 1u, (_decoder_id & 0x00FF'0000u) >> 16u},
          {105u - 1u, (_decoder_id & 0x0000'FF00u) >> 8u},
          {106u - 1u, (_decoder_id & 0x0000'00FFu) >> 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u},
          {8u - 1u, 255},
          {105u - 1u, (_decoder_id & 0xFF00'0000u) >> 24u},
          {106u - 1u, (_decoder_id & 0x00FF'0000u) >> 16u},
          {105u - 1u, (_decoder_id & 0x0000'FF00u) >> 8u},
          {106u - 1u, (_decoder_id & 0x0000'00FFu) >> 0u},
          {105u - 1u, 0u},
          {106u - 1u, 0u}});
}