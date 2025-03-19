#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, transfer_rate_set) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  Packet(pkt);
  auto res = Transmit(pkt);

  ASSERT_EQ(tim, res);

  ASSERT_TRUE(SetTransferRate(TransferRate::Fast));

  tim = builder.timings(TransferRate::Fast);

  res = Transmit(pkt);

  ASSERT_EQ(tim, res);
}

TEST_F(TransmitBaseTest, transfer_rate_no_set_while_busy) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  Packet(pkt);
  auto res = Transmit(5);

  ASSERT_FALSE(SetTransferRate(TransferRate::Fast));

  auto tmp = Transmit(tim.size() - 5);
  std::copy(tmp.begin(), tmp.end(), std::back_inserter(res));

  ASSERT_EQ(res, tim);
}

} // namespace mdu::tx::test
