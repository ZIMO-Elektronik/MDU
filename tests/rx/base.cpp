#include <numeric>
#include "base_test.hpp"
#include "packet_builder.hpp"

using namespace ::testing;

TEST_F(ReceiveBaseTest, shiftIn) {
  bool retval{};

  for (auto i{0u}; i < 7u; ++i) {
    retval = _mock->shiftIn(1u);
    EXPECT_FALSE(retval);
  }

  retval = _mock->shiftIn(1u);
  EXPECT_TRUE(retval);

  retval = _mock->shiftIn(1u);
  EXPECT_FALSE(retval);
}

TEST_F(ReceiveBaseTest, do_not_nack_ackreq_bits_after_reset) {
  Expectation nack_not_sent{
    EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(0))};
  PacketBuilder packet;
  Receive(packet.timingsAckreqOnly());
}

TEST_F(ReceiveBaseTest, single_preamble_bit_is_not_enough_to_set_nack) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(0))};
  PacketBuilder packet;
  packet.preamble(1uz).ackreq();
  Receive(packet.timings());
}

TEST_F(ReceiveBaseTest, two_preamble_bits_are_enough_to_set_nack) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  PacketBuilder packet;
  packet.preamble(2uz).ackreq();
  Receive(packet.timings());
}

TEST_F(ReceiveBaseTest, receive_packet_with_default_transfer_rate) {
  Receive(
    PacketBuilder::makeConfigTransferRatePacket(mdu::TransferRate::Default)
      .timings());
  EXPECT_EQ(size(_mock->_deque), 1u);
}

TEST_F(ReceiveBaseTest, receive_packet_with_fallback_transfer_rate) {
  Receive(
    PacketBuilder::makePingPacket(0u).timings(mdu::TransferRate::Fallback));
  EXPECT_EQ(size(_mock->_deque), 1u);
}

TEST_F(ReceiveBaseTest, receive_packet_with_crc32) {
  std::array<uint8_t, 256uz> zpp_data;
  std::iota(begin(zpp_data), end(zpp_data), 0u);
  Receive(PacketBuilder::makeZppUpdatePacket(0u, zpp_data).timings());
  EXPECT_EQ(size(_mock->_deque), 1u);
}

TEST_F(ReceiveBaseTest, nack_packet_with_crc8_error) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::Ping)
    .data(static_cast<uint32_t>(0))
    .crc8(42u)  // Tinker with CRC8
    .ackreq();
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}

TEST_F(ReceiveBaseTest, nack_and_ack_packet_with_crc32_error) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(6))};
  std::array<uint8_t, 256uz> zpp_data;
  std::iota(begin(zpp_data), end(zpp_data), 0u);
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZppUpdate)
    .data(static_cast<uint32_t>(0))
    .data(zpp_data)
    .crc32(42u)  // Tinker with CRC32
    .ackreq();
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}

TEST_F(ReceiveBaseTest, nack_incomplete_packet) {
  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
  auto packet{PacketBuilder::makePingPacket(0u)};
  auto timings_without_ackreq{packet.timingsWithoutAckreq()};
  // Tinker with packet length
  timings_without_ackreq.erase(end(timings_without_ackreq) - 4,
                               end(timings_without_ackreq));
  Receive(timings_without_ackreq);
  Execute();
  Receive(packet.timingsAckreqOnly(5uz));
}

TEST_F(ReceiveBaseTest, receive_multiple_packets) {
  // This packet will generate an ack
  auto first_packet{PacketBuilder::makePingPacket(0u)};

  // This won't
  auto second_packet{PacketBuilder::makePingPacket(42u)};

  // All in all we will receive 3 ackbits for every first packet, but none from
  // the second packets. This demonstrates that we correctly receive all those
  // packets although we never actually flush the internal ring buffer due to
  // timing constraints.
  auto loop_count{10};
  Expectation nack_sent{
    EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3 * loop_count))};

  auto receive_execute{[this](decltype(first_packet) packet) {
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }};

  for (auto i{0}; i < loop_count; ++i) {
    receive_execute(first_packet);
    receive_execute(second_packet);
  }
}

TEST_F(ReceiveBaseTest, active_after_receiving_one_preamble) {
  EXPECT_FALSE(_mock->active());
  std::vector<uint32_t> times(10u, mdu::timings[0u].one);
  times.push_back(mdu::timings[0u].zero);
  Receive(times);
  EXPECT_TRUE(_mock->active());
}