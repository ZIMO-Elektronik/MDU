#include <numeric>
#include "packet_builder.hpp"
#include "zpp_test.hpp"

using namespace ::testing;

TEST_F(ReceiveZppTest, end_address_check_succeeds) {
  std::array<uint8_t, 64uz> sound_data;
  std::iota(begin(sound_data), end(sound_data), 0u);

  {
    Expectation validate_zpp{EXPECT_CALL(*_mock, zppValid(_, _))
                               .Times(Exactly(1))
                               .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZppValidQueryPacket("SP", 0uz)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation write_zpp{EXPECT_CALL(*_mock, writeZpp(_, _))
                            .Times(Exactly(1))
                            .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZppUpdatePacket(0u, sound_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation end_zpp{EXPECT_CALL(*_mock, endZpp())
                          .Times(Exactly(1))
                          .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZppUpdateEndPacket(0u, size(sound_data))};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}

TEST_F(ReceiveZppTest, end_address_check_fails) {
  std::array<uint8_t, 64uz> sound_data;
  std::iota(begin(sound_data), end(sound_data), 0u);

  {
    Expectation validate_zpp{EXPECT_CALL(*_mock, zppValid(_, _))
                               .Times(Exactly(1))
                               .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZppValidQueryPacket("SP", 0uz)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation write_zpp{EXPECT_CALL(*_mock, writeZpp(_, _))
                            .Times(Exactly(1))
                            .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZppUpdatePacket(0u, sound_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation end_zpp{EXPECT_CALL(*_mock, endZpp()).Times(Exactly(0))};
    Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(3))};
    auto packet{PacketBuilder::makeZppUpdateEndPacket(0u, 42u)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}