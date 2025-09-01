#include "base_test.hpp"

TEST_F(TransmitBaseTest, config_preamble) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timings()};

  auto res{Transmit(packet)};
  ASSERT_EQ(res, timings);

  _cfg.num_preamble = MDU_TX_MIN_PREAMBLE_BITS + 2;
  ASSERT_TRUE(_mock.init(_cfg));

  builder.preamble(_cfg.num_preamble);
  timings = builder.timings();

  res = Transmit(packet);

  ASSERT_EQ(res, timings);
}

TEST_F(TransmitBaseTest, config_ackreq) {
  auto builder{PacketBuilder::makeBusyPacket()};

  auto packet{builder.packet()};
  auto timings{builder.timings()};

  auto res{Transmit(packet)};

  ASSERT_EQ(res, timings);

  _cfg.num_ackreq = MDU_TX_MIN_ACKREQ_BITS + 2;
  ASSERT_TRUE(_mock.init(_cfg));

  builder.ackreq(_cfg.num_ackreq);
  timings = builder.timings();

  res = Transmit(packet);

  ASSERT_EQ(res, timings);
}

TEST_F(TransmitBaseTest, config_no_change_while_busy) {
  auto builder{PacketBuilder::makeBusyPacket()};

  auto packet{builder.packet()};
  auto timings{builder.timings()};

  _mock.packet(packet);

  mdu::tx::Timings res{Transmit(5uz)};

  _cfg.num_preamble = MDU_TX_MIN_PREAMBLE_BITS + 2;
  _cfg.num_ackreq = MDU_TX_MIN_ACKREQ_BITS + 2;

  ASSERT_FALSE(_mock.init(_cfg));

  auto tmp{Transmit(size(timings) - 5)};

  std::ranges::copy(cbegin(tmp), cend(tmp), std::back_inserter(res));

  ASSERT_EQ(res, timings);

  res = Transmit(30);

  timings.clear();
  std::ranges::fill_n(
    std::back_inserter(timings),
    30,
    mdu::timings[std::to_underlying(mdu::TransferRate::Default)].one);

  ASSERT_EQ(res, timings);
}
