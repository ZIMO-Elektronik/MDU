#include "base_test.hpp"

#include <mdu/tx/timings_adapter.hpp>

TEST(TimingsAdapterTest, packet) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timingsWithoutAckreq()};

  mdu::tx::TimingsAdapter a{packet, {}, mdu::TransferRate::Default};

  mdu::tx::Timings result;
  std::ranges::copy(cbegin(a), cend(a), std::back_inserter(result));

  ASSERT_EQ(result, timings);
}

TEST(TimingsAdapterTest, config_preamble) {
  auto builder{PacketBuilder::makeBusyPacket()};
  builder.preamble(16uz);
  auto packet{builder.packet()};
  auto timings{builder.timingsWithoutAckreq()};

  mdu::tx::TimingsAdapter a{packet, {16uz}, mdu::TransferRate::Default};

  mdu::tx::Timings result;
  std::ranges::copy(cbegin(a), cend(a), std::back_inserter(result));

  ASSERT_EQ(result, timings);
}

TEST(TimingsAdapterTest, transfer_rate) {
  auto builder{PacketBuilder::makeBusyPacket()};
  auto packet{builder.packet()};
  auto timings{builder.timingsWithoutAckreq(mdu::TransferRate::Fast)};

  mdu::tx::TimingsAdapter a{packet, {}, mdu::TransferRate::Fast};

  mdu::tx::Timings result;
  std::ranges::copy(cbegin(a), cend(a), std::back_inserter(result));

  ASSERT_EQ(result, timings);
}
