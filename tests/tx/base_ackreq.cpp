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

TEST_F(TransmitBaseTest, ackreq_methods) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  Config cfg{};

  Packet(pkt);
  Transmit(tim.size() - cfg.num_ackreq);

  EXPECT_CALL(_mock, ackreqBegin()).Times(1);
  Transmit(1);

  Transmit(1);

  {
    ::testing::InSequence s;

    EXPECT_CALL(_mock, ackreqChannel1(2));
    EXPECT_CALL(_mock, ackreqChannel1(3));
    EXPECT_CALL(_mock, ackreqChannel1(4));
  }
  Transmit(3);

  Transmit(1);

  {
    ::testing::InSequence s;

    EXPECT_CALL(_mock, ackreqChannel2(6));
    EXPECT_CALL(_mock, ackreqChannel2(7));
    EXPECT_CALL(_mock, ackreqChannel2(8));
  }

  Transmit(3);

  EXPECT_CALL(_mock, ackreqEnd());
  Transmit(tim.size() - (cfg.num_ackreq - 8));
}

} // namespace mdu::tx::test