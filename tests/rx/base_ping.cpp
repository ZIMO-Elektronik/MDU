#include "base_test.hpp"
#include "utility.hpp"

using namespace ::testing;

TEST_F(ReceiveBaseTest, initial_state_of_selected_is_true) {
  EXPECT_TRUE(base_->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_short_all) {
  base_->select(false);
  EXPECT_FALSE(base_->selected());
  Expectation ack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
  auto packet{make_ping_packet(0u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(base_->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_short_to_decoder_id) {
  base_->select(false);
  EXPECT_FALSE(base_->selected());
  Expectation ack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
  auto packet{make_ping_packet(decoder_id_ >> 24u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(base_->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_long_all) {
  base_->select(false);
  EXPECT_FALSE(base_->selected());
  Expectation ack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
  auto packet{make_ping_packet(0u, 0u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(base_->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_long_to_serial_number) {
  base_->select(false);
  EXPECT_FALSE(base_->selected());
  Expectation ack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
  auto packet{make_ping_packet(serial_number_, 0u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(base_->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_long_to_decoder_id) {
  base_->select(false);
  EXPECT_FALSE(base_->selected());
  Expectation ack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
  auto packet{make_ping_packet(0u, decoder_id_)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(base_->selected());
}

TEST_F(ReceiveBaseTest, select_with_ping_long_to_serial_number_and_decoder_id) {
  base_->select(false);
  EXPECT_FALSE(base_->selected());
  Expectation ack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
  auto packet{make_ping_packet(serial_number_, decoder_id_)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_TRUE(base_->selected());
}

TEST_F(ReceiveBaseTest, deselect_with_ping_long_to_different_serial_number) {
  EXPECT_TRUE(base_->selected());
  auto packet{make_ping_packet(42u, 0u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_FALSE(base_->selected());
}

TEST_F(ReceiveBaseTest, deselect_with_ping_long_to_different_decoder_id) {
  EXPECT_TRUE(base_->selected());
  auto packet{make_ping_packet(0u, 42u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_FALSE(base_->selected());
}

TEST_F(ReceiveBaseTest,
       deselect_with_ping_long_to_different_serial_number_and_decoder_id) {
  EXPECT_TRUE(base_->selected());
  auto packet{make_ping_packet(42u, 43u)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
  EXPECT_FALSE(base_->selected());
}