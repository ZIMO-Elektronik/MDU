#include "base_test.hpp"

TEST_F(TransmitBaseTest, transmit_start_bit) {
  auto packet{PacketBuilder::makeBusyPacket().packet()};
  auto timings{Transmit(packet)};
  ASSERT_EQ(timings[_cfg.num_preamble],
            mdu::timings[std::to_underlying(mdu::TransferRate::Default)].zero);
}

TEST_F(TransmitBaseTest, transmit_bytes) {
  auto packet{PacketBuilder::makeBusyPacket().packet()};
  auto timings{Transmit(packet)};
  EXPECT_EQ(timings, mdu::tx::bytes2timings(packet, _cfg));
}
