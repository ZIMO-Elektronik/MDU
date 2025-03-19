#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, packet_end_bit) {
  auto pkt = PacketBuilder::makeBusyPacket().packet();

  auto res = Transmit(pkt);

  Timings::size_type offset = MDU_TX_MIN_PREAMBLE_BITS // Preamble
                              + pkt.size() * 9;        // Bytes

  ASSERT_TRUE(EvalStopBit(res, offset));
}

TEST_F(TransmitBaseTest, ackreq) {
  auto pkt = PacketBuilder::makeBusyPacket().packet();

  auto res = Transmit(pkt);

  Timings::size_type offset = MDU_TX_MIN_PREAMBLE_BITS // Preamble
                              + pkt.size() * 9         // Bytes
                              + 1;                     // StopBit

  ASSERT_TRUE(EvalAckReq(res, offset));
}

TEST_F(TransmitBaseTest, idle_after_ackreq) {
  auto pkt = PacketBuilder::makeBusyPacket().packet();

  auto res = Transmit(30);
  Timings eval{};
  std::ranges::fill_n(std::back_inserter(eval),
                      30,
                      timings[std::to_underlying(TransferRate::Default)].one);

  ASSERT_EQ(res, eval);
}

} // namespace mdu::tx::test