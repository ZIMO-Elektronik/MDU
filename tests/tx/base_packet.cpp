#include "base_test.hpp"

TEST_F(TransmitBaseTest, single_packet) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timings()};

  ASSERT_EQ(Transmit(packet), timings);
}

TEST_F(TransmitBaseTest, multiple_packets) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timings()};

  ASSERT_EQ(Transmit(packet), timings);
  ASSERT_EQ(Transmit(packet), timings);
  ASSERT_EQ(Transmit(packet), timings);
}

TEST_F(TransmitBaseTest, no_new_packet_during_transmission) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timings()};

  // Transmit first 15 bits
  ASSERT_TRUE(_mock.packet(packet));
  mdu::tx::Timings res{Transmit(15uz)};

  // Can't push new packet while transmission ongoing
  ASSERT_FALSE(_mock.packet(packet));

  // Transmit residual
  auto tmp{Transmit(size(timings) - 15uz)};
  std::ranges::copy(cbegin(tmp), cend(tmp), std::back_inserter(res));
  ASSERT_EQ(res, timings);
}
