#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, transmit_start_bit) {
  Enqueue4ByteDummy();
  SkipPreamble();
  ASSERT_EQ(Transmit(),
            mdu::timings[std::to_underlying(TransferRate::Default)].zero);
}

TEST_F(TransmitBaseTest, transmit_byte) {
  Enqueue4ByteDummy();
  SkipPreamble();
  std::array<uint16_t, 9> bte = TransmitByte();
  ASSERT_EQ(Timings2Byte(bte), 0x01u);
}

TEST_F(TransmitBaseTest, transmit_bytes) {
  Enqueue4ByteDummy();
  SkipPreamble();
  std::array<uint16_t, 9> bte = TransmitByte();
  ASSERT_EQ(Timings2Byte(bte), 0x01u);
  bte = TransmitByte();
  ASSERT_EQ(Timings2Byte(bte), 0x02u);
  bte = TransmitByte();
  ASSERT_EQ(Timings2Byte(bte), 0x03u);
  bte = TransmitByte();
  ASSERT_EQ(Timings2Byte(bte), 0x04u);
}

} // namespace mdu::tx::test