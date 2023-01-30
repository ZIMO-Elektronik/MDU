#include <numeric>
#include "utility.hpp"
#include "zpp_test.hpp"

using namespace ::testing;

TEST_F(ReceiveZppTest, end_address_check_succeeds) {
  std::array<uint8_t, 64u> sound_data;
  std::iota(begin(sound_data), end(sound_data), 0u);

  Expectation write_sound{EXPECT_CALL(*base_, writeZpp(_, _))
                            .Times(Exactly(1))
                            .WillRepeatedly(Return(true))};

  {
    auto packet{make_zpp_update_packet(0u, sound_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  Expectation end_sound{EXPECT_CALL(*base_, endZpp())
                          .Times(Exactly(1))
                          .WillRepeatedly(Return(true))};

  {
    auto packet{make_zpp_update_end_packet(0u, size(sound_data))};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}

TEST_F(ReceiveZppTest, end_address_check_fails) {
  std::array<uint8_t, 64u> sound_data;
  std::iota(begin(sound_data), end(sound_data), 0u);

  {
    Expectation write_sound{EXPECT_CALL(*base_, writeZpp(_, _))
                              .Times(Exactly(1))
                              .WillRepeatedly(Return(true))};
    auto packet{make_zpp_update_packet(0u, sound_data)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation end_sound{EXPECT_CALL(*base_, endZpp()).Times(Exactly(0))};
    Expectation nack_sent{EXPECT_CALL(*base_, ackbit(100u)).Times(Exactly(3))};
    auto packet{make_zpp_update_end_packet(0u, 42u)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}