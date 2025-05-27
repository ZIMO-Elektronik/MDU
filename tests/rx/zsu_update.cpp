#include <numeric>
#include "../packet_builder.hpp"
#include "zsu_test.hpp"

using namespace ::testing;

TEST_F(ReceiveZsuTest, write_single_update_packet) {
  Expectation write_zsu{EXPECT_CALL(*_mock, writeZsu(0u, _))
                          .Times(Exactly(1))
                          .WillRepeatedly(Return(true))};
  std::array<uint8_t, 64uz> zsu_data;
  std::iota(begin(zsu_data), end(zsu_data), 0u);
  auto packet{PacketBuilder::makeZsuUpdatePacket(0u, zsu_data)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}

TEST_F(ReceiveZsuTest, write_two_consecutive_update_packets) {
  Expectation write_zsu{EXPECT_CALL(*_mock, writeZsu(_, _))
                          .Times(Exactly(2))
                          .WillRepeatedly(Return(true))};
  std::array<uint8_t, 64uz> zsu_data;
  std::iota(begin(zsu_data), end(zsu_data), 0u);

  // Write packet to address 42
  {
    auto packet{PacketBuilder::makeZsuUpdatePacket(42u, zsu_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  // Write packet to address 42 + 64
  {
    auto packet{
      PacketBuilder::makeZsuUpdatePacket(42u + size(zsu_data), zsu_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}

TEST_F(ReceiveZsuTest, dont_write_to_same_update_address_twice) {
  Expectation write_zsu{EXPECT_CALL(*_mock, writeZsu(0u, _))
                          .Times(Exactly(1))
                          .WillRepeatedly(Return(true))};
  std::array<uint8_t, 64uz> zsu_data;
  std::iota(begin(zsu_data), end(zsu_data), 0u);

  // Packet to address 64 is only written once
  for (auto i{0u}; i < 2u; ++i) {
    auto packet{PacketBuilder::makeZsuUpdatePacket(0u, zsu_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}

TEST_F(ReceiveZsuTest, nack_update_addresses_greater_than_expected) {
  std::array<uint8_t, 64uz> zsu_data;
  std::iota(begin(zsu_data), end(zsu_data), 0u);

  // Write packet to address 0
  {
    Expectation write_zsu{EXPECT_CALL(*_mock, writeZsu(_, _))
                            .Times(Exactly(1))
                            .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZsuUpdatePacket(0u, zsu_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  // Try to write packet to address 64 + 42
  {
    Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
    auto packet{
      PacketBuilder::makeZsuUpdatePacket(0u + size(zsu_data) + 42u, zsu_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}
