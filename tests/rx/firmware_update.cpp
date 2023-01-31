#include <numeric>
#include "firmware_test.hpp"
#include "utility.hpp"

using namespace ::testing;

TEST_F(ReceiveFirmwareTest, write_single_update_packet) {
  Expectation write_firmware{EXPECT_CALL(*base_, writeFirmware(0u, _))
                               .Times(Exactly(1))
                               .WillRepeatedly(Return(true))};
  std::array<uint8_t, 64uz> firmware_data;
  std::iota(begin(firmware_data), end(firmware_data), 0u);
  auto packet{make_firmware_update_packet(0u, firmware_data)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}

TEST_F(ReceiveFirmwareTest, write_two_consecutive_update_packets) {
  Expectation write_firmware{EXPECT_CALL(*base_, writeFirmware(_, _))
                               .Times(Exactly(2))
                               .WillRepeatedly(Return(true))};
  std::array<uint8_t, 64uz> firmware_data;
  std::iota(begin(firmware_data), end(firmware_data), 0u);

  // Write packet to address 42
  {
    auto packet{make_firmware_update_packet(42u, firmware_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  // Write packet to address 42 + 64
  {
    auto packet{
      make_firmware_update_packet(42u + size(firmware_data), firmware_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}

TEST_F(ReceiveFirmwareTest, dont_write_to_same_update_address_twice) {
  Expectation write_firmware{EXPECT_CALL(*base_, writeFirmware(0u, _))
                               .Times(Exactly(1))
                               .WillRepeatedly(Return(true))};
  std::array<uint8_t, 64uz> firmware_data;
  std::iota(begin(firmware_data), end(firmware_data), 0u);

  // Packet to address 64 is only written once
  for (auto i{0u}; i < 2u; ++i) {
    auto packet{make_firmware_update_packet(0u, firmware_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}

TEST_F(ReceiveFirmwareTest, nack_update_addresses_greater_than_expected) {
  std::array<uint8_t, 64uz> firmware_data;
  std::iota(begin(firmware_data), end(firmware_data), 0u);

  // Write packet to address 0
  {
    Expectation write_firmware{EXPECT_CALL(*base_, writeFirmware(_, _))
                                 .Times(Exactly(1))
                                 .WillRepeatedly(Return(true))};
    auto packet{make_firmware_update_packet(0u, firmware_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  // Try to write packet to address 64 + 42
  {
    Expectation nack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
    auto packet{make_firmware_update_packet(0u + size(firmware_data) + 42u,
                                            firmware_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}