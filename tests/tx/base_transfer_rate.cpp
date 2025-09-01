#include "base_test.hpp"

TEST_F(TransmitBaseTest, transfer_rate_set) {
  auto builder{PacketBuilder::makeBusyPacket()};

  auto packet{builder.packet()};
  auto timings{builder.timings()};

  _mock.packet(packet);
  auto res{Transmit(packet)};

  ASSERT_EQ(timings, res);

  ASSERT_TRUE(_mock.setTransferRate(mdu::TransferRate::Fast));

  timings = builder.timings(mdu::TransferRate::Fast);

  res = Transmit(packet);

  ASSERT_EQ(timings, res);
}

TEST_F(TransmitBaseTest, transfer_rate_no_set_while_busy) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto packet{builder.packet()};
  auto timings{builder.timings()};

  _mock.packet(packet);
  auto res{Transmit(5uz)};

  ASSERT_FALSE(_mock.setTransferRate(mdu::TransferRate::Fast));

  auto tmp{Transmit(size(timings) - 5uz)};
  std::copy(cbegin(tmp), cend(tmp), std::back_inserter(res));

  ASSERT_EQ(res, timings);
}
