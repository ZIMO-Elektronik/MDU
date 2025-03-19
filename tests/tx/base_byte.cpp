#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, transmit_start_bit) {
  auto pkt = PacketBuilder::makeBusyPacket().packet();

  auto res = Transmit(pkt);

  Timings::size_type offset = MDU_TX_MIN_PREAMBLE_BITS;

  ASSERT_TRUE(EvalStartBit(res, offset));
}

TEST_F(TransmitBaseTest, transmit_byte) {
  auto pkt = PacketBuilder::makeBusyPacket().packet();

  auto res = Transmit(pkt);

  Timings::size_type offset = MDU_TX_MIN_PREAMBLE_BITS + 1;

  ASSERT_TRUE(EvalByte(res, pkt.front(), offset));
}

TEST_F(TransmitBaseTest, transmit_bytes) {
  auto pkt = PacketBuilder::makeBusyPacket().packet();

  auto res = Transmit(pkt);

  Timings::size_type offset = MDU_TX_MIN_PREAMBLE_BITS;

  for (auto it : pkt) {
    ASSERT_TRUE(EvalStartBit(res, offset));
    ASSERT_TRUE(EvalByte(res, it, offset));
  }
}

} // namespace mdu::tx::test