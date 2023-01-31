#include <numeric>
#include "utility.hpp"
#include "zpp_test.hpp"

using namespace ::testing;

TEST_F(ReceiveZppTest, exit_when_nothing_was_written) {
  Expectation exit_zpp{EXPECT_CALL(*base_, exitZpp(false)).Times(Exactly(1))};
  auto packet{make_zpp_exit_packet()};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}

TEST_F(ReceiveZppTest, exit_when_end_address_check_succeeds) {
  std::array<uint8_t, 64uz> zpp_data;
  std::iota(begin(zpp_data), end(zpp_data), 0u);

  Expectation write_zpp{EXPECT_CALL(*base_, writeZpp(_, _))
                          .Times(Exactly(1))
                          .WillRepeatedly(Return(true))};

  {
    auto packet{make_zpp_update_packet(0u, zpp_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  Expectation end_zpp{EXPECT_CALL(*base_, endZpp())
                        .Times(Exactly(1))
                        .WillRepeatedly(Return(true))};

  {
    auto packet{make_zpp_update_end_packet(0u, size(zpp_data))};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation exit_zpp{EXPECT_CALL(*base_, exitZpp(false)).Times(Exactly(1))};
    auto packet{make_zpp_exit_packet()};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}

TEST_F(ReceiveZppTest, erase_zpp_when_end_address_check_fails) {
  std::array<uint8_t, 64uz> zpp_data;
  std::iota(begin(zpp_data), end(zpp_data), 0u);

  Expectation nack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(6))};

  {
    Expectation write_zpp{EXPECT_CALL(*base_, writeZpp(_, _))
                            .Times(Exactly(1))
                            .WillRepeatedly(Return(true))};
    auto packet{make_zpp_update_packet(0u, zpp_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation end_zpp{EXPECT_CALL(*base_, endZpp()).Times(Exactly(0))};
    auto packet{make_zpp_update_end_packet(0u, 42u)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation exit_zpp{EXPECT_CALL(*base_, exitZpp(false)).Times(Exactly(0))};
    Expectation erase_zpp{EXPECT_CALL(*base_, eraseZpp(0u, size(zpp_data)))
                            .Times(Exactly(1))
                            .WillRepeatedly(Return(true))};
    auto packet{make_zpp_exit_packet()};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}