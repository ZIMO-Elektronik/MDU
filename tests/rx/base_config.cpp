#include "base_test.hpp"
#include "utility.hpp"

using namespace ::testing;

TEST_F(ReceiveBaseTest, receive_packet_after_changing_transfer_rate) {
  auto cfg{cfg_};
  cfg.transfer_rate = mdu::TransferRate::Fast;
  base_ = std::make_unique<BaseMock>(cfg);

  {
    auto packet{make_config_transfer_rate_packet(mdu::TransferRate::Default)};
    Receive(packet.timings());
    EXPECT_EQ(size(base_->queue_), 1u);
    Execute();
  }

  {
    auto packet{make_config_transfer_rate_packet(mdu::TransferRate::Fast)};
    Receive(packet.timings());
    EXPECT_EQ(size(base_->queue_), 1u);
    Execute();
  }

  {
    auto packet{make_ping_packet(0u, 0u)};
    Receive(packet.timings(mdu::TransferRate::Fast));
    EXPECT_EQ(size(base_->queue_), 1u);
  }
}

TEST_F(ReceiveBaseTest,
       nack_config_transfer_rate_packet_with_unsupported_transfer_rate) {
  Expectation nack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
  auto packet{make_config_transfer_rate_packet(mdu::TransferRate::Fast)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}