#include "../packet_builder.hpp"
#include "base_test.hpp"

using namespace ::testing;

TEST_F(ReceiveBaseTest, receive_packet_after_changing_transfer_rate) {
  auto cfg{_cfg};
  cfg.transfer_rate = mdu::TransferRate::Fast;
  _mock = std::make_unique<BaseMock>(cfg);

  {
    auto packet{
      PacketBuilder::makeConfigTransferRatePacket(mdu::TransferRate::Default)};
    Receive(packet.timings());
    EXPECT_EQ(size(_mock->_deque), 1u);
    Execute();
  }

  {
    auto packet{
      PacketBuilder::makeConfigTransferRatePacket(mdu::TransferRate::Fast)};
    Receive(packet.timings());
    EXPECT_EQ(size(_mock->_deque), 1u);
    Execute();
  }

  {
    auto packet{PacketBuilder::makePingPacket(0u, 0u)};
    Receive(packet.timings(mdu::TransferRate::Fast));
    EXPECT_EQ(size(_mock->_deque), 1u);
  }
}

TEST_F(ReceiveBaseTest,
       nack_config_transfer_rate_packet_with_unsupported_transfer_rate) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{
    PacketBuilder::makeConfigTransferRatePacket(mdu::TransferRate::Fast)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}