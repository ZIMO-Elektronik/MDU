#include "base_test.hpp"

TEST_F(TransmitBaseTest, send_preamble) {
  auto packet{PacketBuilder::makeBusyPacket().packet()};
  auto timings{Transmit(packet)};
  ASSERT_TRUE(std::all_of(
    cbegin(timings), cbegin(timings) + _cfg.num_preamble, [](auto t) {
      return t ==
             mdu::timings[std::to_underlying(mdu::TransferRate::Default)].one;
    }));
}
