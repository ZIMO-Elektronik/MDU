#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, config_preamble) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  auto res = Transmit(pkt);

  ASSERT_EQ(res, tim);

  Config cfg{};
  cfg.num_preamble = MDU_TX_MIN_PREAMBLE_BITS + 2;
  Init(cfg);

  builder.preamble(cfg.num_preamble);
  tim = builder.timings();

  res = Transmit(pkt, cfg);

  ASSERT_EQ(res, tim);
}

TEST_F(TransmitBaseTest, config_ackreq) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  auto res = Transmit(pkt);

  ASSERT_EQ(res, tim);

  Config cfg{};
  cfg.num_ackreq = MDU_TX_MIN_ACKREQ_BITS + 2;
  Init(cfg);

  builder.ackreq(cfg.num_ackreq);
  tim = builder.timings();

  res = Transmit(pkt, cfg);

  ASSERT_EQ(res, tim);
}

TEST_F(TransmitBaseTest, config_polarity) {}

TEST_F(TransmitBaseTest, config_no_change_while_busy) {
  auto builder = PacketBuilder::makeBusyPacket();

  auto pkt = builder.packet();
  auto tim = builder.timings();

  Packet(pkt);

  Timings res = Transmit(5);

  Config cfg{};
  cfg.num_preamble = MDU_TX_MIN_PREAMBLE_BITS + 2;
  cfg.num_ackreq = MDU_TX_MIN_ACKREQ_BITS + 2;

  Init(cfg);

  auto tmp = Transmit(tim.size() - 5);

  std::ranges::copy(tmp.begin(), tmp.end(), std::back_inserter(res));

  ASSERT_EQ(res, tim);

  res = Transmit(30);

  tim.clear();
  std::ranges::fill_n(std::back_inserter(tim),
                      30,
                      timings[std::to_underlying(TransferRate::Default)].one);

  ASSERT_EQ(res, tim);
}

} // namespace mdu::tx::test