#include <mdu/tx/timings.hpp>
#include "base_test.hpp"

TEST(TimingsTest, packet) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timings()};

  mdu::tx::Timings t{
    mdu::tx::bytes2timings(packet, {}, mdu::TransferRate::Default)};

  mdu::tx::Timings result;
  std::ranges::copy(cbegin(t), cend(t), std::back_inserter(result));

  ASSERT_EQ(result, timings);
}

TEST(TimingsTest, config_preamble) {
  auto builder{PacketBuilder::makeBusyPacket()};
  builder.preamble(16uz);
  auto packet{builder.packet()};
  auto timings{builder.timings()};

  mdu::tx::Timings t{mdu::tx::bytes2timings(
    packet, {.num_preamble = 16uz}, mdu::TransferRate::Default)};

  mdu::tx::Timings result;
  std::ranges::copy(cbegin(t), cend(t), std::back_inserter(result));

  ASSERT_EQ(result, timings);
}

TEST(TimingsTest, transfer_rate) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timings(mdu::TransferRate::Fast)};

  mdu::tx::Timings t{
    mdu::tx::bytes2timings(packet, {}, mdu::TransferRate::Fast)};

  mdu::tx::Timings result;
  std::ranges::copy(cbegin(t), cend(t), std::back_inserter(result));

  ASSERT_EQ(result, timings);
}
