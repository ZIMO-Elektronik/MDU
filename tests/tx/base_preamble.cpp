#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, send_preamble) {
  auto pkt = PacketBuilder::makeBusyPacket().packet();

  auto res = Transmit(pkt);

  Timings::size_type offset{0};

  ASSERT_TRUE(EvalPreamble(res, offset, TransferRate::Default));
}

} // namespace mdu::tx::test