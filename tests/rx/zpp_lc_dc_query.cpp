#include "../packet_builder.hpp"
#include "zpp_test.hpp"

using namespace testing;

namespace std {

// Need operator overload for EXPECT_CALL
bool operator==(span<uint8_t const, 4uz> lhs, span<uint8_t const, 4uz> rhs) {
  return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
}

} // namespace std

TEST_F(ReceiveZppTest, load_code_valid) {
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
    Expectation load_code_valid{
      EXPECT_CALL(*_mock, loadCodeValid({_developer_code}))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true))};
    auto packet{PacketBuilder::makeZppLcDcQueryPacket(_developer_code)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}
