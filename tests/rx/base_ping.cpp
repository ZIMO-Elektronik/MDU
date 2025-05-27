#include "../packet_builder.hpp"
#include "base_test.hpp"

using namespace ::testing;

TEST_F(ReceiveBaseTest, initial_state_of_selected_is_true) {
  EXPECT_TRUE(_mock->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_short_all) {
  _mock->select(false);
  EXPECT_FALSE(_mock->selected());
  Expectation ack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{PacketBuilder::makePingPacket(0u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(_mock->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_short_to_decoder_id) {
  _mock->select(false);
  EXPECT_FALSE(_mock->selected());
  Expectation ack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{PacketBuilder::makePingPacket(_decoder_id >> 24u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(_mock->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_long_all) {
  _mock->select(false);
  EXPECT_FALSE(_mock->selected());
  Expectation ack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{PacketBuilder::makePingPacket(0u, 0u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(_mock->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_long_to_serial_number) {
  _mock->select(false);
  EXPECT_FALSE(_mock->selected());
  Expectation ack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{PacketBuilder::makePingPacket(_serial_number, 0u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(_mock->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_long_to_decoder_id) {
  _mock->select(false);
  EXPECT_FALSE(_mock->selected());
  Expectation ack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{PacketBuilder::makePingPacket(0u, _decoder_id)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(_mock->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_long_to_serial_number_and_decoder_id) {
  _mock->select(false);
  EXPECT_FALSE(_mock->selected());
  Expectation ack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{PacketBuilder::makePingPacket(_serial_number, _decoder_id)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(_mock->selected());
}

TEST_F(ReceiveBaseTest, deselect_with_ping_long_to_different_serial_number) {
  EXPECT_TRUE(_mock->selected());
  auto packet{PacketBuilder::makePingPacket(42u, 0u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_FALSE(_mock->selected());
}

TEST_F(ReceiveBaseTest, deselect_with_ping_long_to_different_decoder_id) {
  EXPECT_TRUE(_mock->selected());
  auto packet{PacketBuilder::makePingPacket(0u, 42u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_FALSE(_mock->selected());
}

TEST_F(ReceiveBaseTest,
       deselect_with_ping_long_to_different_serial_number_and_decoder_id) {
  EXPECT_TRUE(_mock->selected());
  auto packet{PacketBuilder::makePingPacket(42u, 43u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_FALSE(_mock->selected());
}
