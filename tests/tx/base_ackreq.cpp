#include "base_test.hpp"

TEST_F(TransmitBaseTest, packet_end_bit) {
  auto packet{PacketBuilder::makeBusyPacket().packet()};
  auto timings{Transmit(packet)};
  ASSERT_EQ(timings[static_cast<mdu::tx::Timings::size_type>(
              _cfg.num_preamble + // Preamble
              size(packet) * 9 -  // Bytes
              1)],                // StopBit
            mdu::timings[std::to_underlying(mdu::TransferRate::Default)].zero);
}

TEST_F(TransmitBaseTest, ackreq) {
  auto packet{PacketBuilder::makeBusyPacket().packet()};
  auto timings{Transmit(packet)};
  ASSERT_TRUE(std::all_of(
    cbegin(timings) + _cfg.num_preamble + // Preamble
      size(packet) * 9                    // Bytes
      + 1,                                // StopBit
    cend(timings),
    [](auto t) {
      return t == mdu::timings[std::to_underlying(mdu::TransferRate::Default)]
                    .ackreq;
    }));
}

TEST_F(TransmitBaseTest, idle_after_ackreq) {
  auto packet{PacketBuilder::makeBusyPacket().packet()};

  Transmit(packet);

  auto timings{Transmit(30uz)};
  mdu::tx::Timings eval{};
  std::ranges::fill_n(
    std::back_inserter(eval),
    30,
    mdu::timings[std::to_underlying(mdu::TransferRate::Default)].one);

  ASSERT_EQ(timings, eval);
}

TEST_F(TransmitBaseTest, ackreq_methods) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timings()};

  _mock.packet(packet);
  Transmit(size(timings) - _cfg.num_ackreq);

  EXPECT_CALL(_mock, ackreqBegin()).Times(1);
  Transmit(1uz);

  Transmit(1uz);

  {
    testing::InSequence s;
    EXPECT_CALL(_mock, ackreqChannel1(2uz));
    EXPECT_CALL(_mock, ackreqChannel1(3uz));
    EXPECT_CALL(_mock, ackreqChannel1(4uz));
  }
  Transmit(3uz);

  Transmit(1uz);

  {
    testing::InSequence s;
    EXPECT_CALL(_mock, ackreqChannel2(6uz));
    EXPECT_CALL(_mock, ackreqChannel2(7uz));
    EXPECT_CALL(_mock, ackreqChannel2(8uz));
  }

  Transmit(3uz);

  EXPECT_CALL(_mock, ackreqEnd());
  Transmit(size(timings) - (_cfg.num_ackreq - 8uz));
}
