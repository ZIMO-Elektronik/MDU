#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, start_ackreq) {
  Enqueue4ByteDummy();
  SkipPreamble();
  Transmit4Bytes();
  ASSERT_EQ(Transmit(),
            timings[std::to_underlying(TransferRate::Default)].ackreq);
}

TEST_F(TransmitBaseTest, ackreq_bits) {
  Enqueue4ByteDummy();
  SkipPreamble();
  Transmit4Bytes();
  auto ackreq = TransmitByte();
  int cnt = 0;
  for (auto it : ackreq) {
    ASSERT_EQ(it, timings[std::to_underlying(TransferRate::Default)].ackreq)
      << "Expected " << MDU_TX_MIN_ACKREQ_BITS << "ACKreq bits but only got "
      << cnt;
    cnt++;
  }
}

TEST_F(TransmitBaseTest, idle_after_ackreq) {
  Enqueue4ByteDummy();
  SkipPreamble();
  Transmit4Bytes();
  TransmitACKreq();

  ASSERT_EQ(Transmit(), timings[std::to_underlying(TransferRate::Default)].one);
}

} // namespace mdu::tx::test