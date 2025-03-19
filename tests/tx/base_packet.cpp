#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, single_packet) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  auto res = Transmit(pkt);

  ASSERT_EQ(res, tim);
}

TEST_F(TransmitBaseTest, multiple_packets) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  auto res = Transmit(pkt);

  ASSERT_EQ(res, tim);

  res = Transmit(pkt);

  ASSERT_EQ(res, tim);

  res = Transmit(pkt);

  ASSERT_EQ(res, tim);
}

TEST_F(TransmitBaseTest, no_new_packet_during_transfer) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  ASSERT_TRUE(Packet(pkt));

  Timings res = Transmit(15);

  ASSERT_FALSE(Packet(pkt));

  auto tmp = Transmit(tim.size() - 15);

  std::ranges::copy(tmp.begin(), tmp.end(), std::back_inserter(res));

  ASSERT_EQ(res, tim);
}

} // namespace mdu::tx::test