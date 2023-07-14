#include "base_test.hpp"
#include "utility.hpp"

using namespace ::testing;

TEST_F(ReceiveBaseTest, ignore_busy_when_not_busy) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(0))};
  Receive(make_busy_packet().timings());
}

TEST_F(ReceiveBaseTest, ack_busy_packet_when_busy) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto config_packet{
    make_config_transfer_rate_packet(mdu::TransferRate::Default)};
  auto busy_packet{make_busy_packet()};
  Receive(config_packet.timings());
  Receive(busy_packet.timingsWithoutAckreq());
  Execute();
  Receive(busy_packet.timingsAckreqOnly());
}

TEST_F(ReceiveBaseTest, do_not_nack_busy_packet_when_busy) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(0))};
  auto config_packet{
    make_config_transfer_rate_packet(mdu::TransferRate::Default)};
  auto busy_packet{make_busy_packet()};
  Receive(config_packet.timings());
  Receive(busy_packet.timingsWithoutAckreq());
  Execute();
  Receive(busy_packet.timingsAckreqOnly(5u));
}

TEST_F(ReceiveBaseTest, nack_other_packets_when_busy) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{make_config_transfer_rate_packet(mdu::TransferRate::Default)};
  Receive(packet.timings());
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly(5u));
}