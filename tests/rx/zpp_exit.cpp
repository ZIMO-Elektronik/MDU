#include <numeric>
#include "../packet_builder.hpp"
#include "zpp_test.hpp"

using namespace ::testing;

TEST_F(ReceiveZppTest, exit_when_nothing_was_written) {
  Expectation exit_zpp{EXPECT_CALL(*_mock, exitZpp(false)).Times(Exactly(1))};
  auto packet{PacketBuilder::makeZppExitPacket()};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}

TEST_F(ReceiveZppTest, exit_when_end_address_check_succeeds) {
  std::array<uint8_t, 64uz> zpp_data;
  std::iota(begin(zpp_data), end(zpp_data), 0u);

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
    auto packet{PacketBuilder::makeZppUpdatePacket(0u, zpp_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation end_zpp{EXPECT_CALL(*_mock, endZpp())
                          .Times(Exactly(1))
                          .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZppUpdateEndPacket(0u, size(zpp_data))};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation exit_zpp{EXPECT_CALL(*_mock, exitZpp(false)).Times(Exactly(1))};
    auto packet{PacketBuilder::makeZppExitPacket()};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}

TEST_F(ReceiveZppTest, erase_zpp_when_end_address_check_fails) {
  std::array<uint8_t, 64uz> zpp_data;
  std::iota(begin(zpp_data), end(zpp_data), 0u);

  Expectation nack_sent{EXPECT_CALL(*_mock, ackbit(100u)).Times(Exactly(6))};

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
    auto packet{PacketBuilder::makeZppUpdatePacket(0u, zpp_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation end_zpp{EXPECT_CALL(*_mock, endZpp()).Times(Exactly(0))};
    auto packet{PacketBuilder::makeZppUpdateEndPacket(0u, 42u)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation exit_zpp{EXPECT_CALL(*_mock, exitZpp(false)).Times(Exactly(0))};
    Expectation erase_zpp{EXPECT_CALL(*_mock, eraseZpp(0u, size(zpp_data)))
                            .Times(Exactly(1))
                            .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZppExitPacket()};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}
