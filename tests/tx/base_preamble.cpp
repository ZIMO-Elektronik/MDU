#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, send_preamble) {
  Enqueue4ByteDummy();
  for (int i = 0; i < MDU_TX_MIN_PREAMBLE_BITS; i++) {
    ASSERT_EQ(Transmit(),
              mdu::timings[std::to_underlying(TransferRate::Default)].one)
      << "Expected " << MDU_TX_MIN_PREAMBLE_BITS << ", but got only " << i;
  }
  ASSERT_NE(Transmit(),
            mdu::timings[std::to_underlying(TransferRate::Default)].one);
}

} // namespace mdu::tx::test