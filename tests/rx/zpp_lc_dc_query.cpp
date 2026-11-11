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
  Expectation validate_zpp{EXPECT_CALL(*_mock, zppValid(_, _))
                             .Times(Exactly(1))
                             .WillRepeatedly(Return(true))};
  auto packet{PacketBuilder::makeZppValidQueryPacket("SP", 0uz)};
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());

  Expectation load_code_valid{
    EXPECT_CALL(*_mock, loadCodeValid({_developer_code}))
      .Times(Exactly(1))
      .WillRepeatedly(Return(true))};
  packet = PacketBuilder::makeZppLcDcQueryPacket(_developer_code);
  Receive(packet.timingsWithoutAckreq());
  Execute();
  Receive(packet.timingsAckreqOnly());
}

TEST_F(ReceiveZppTest, exceed_zpp_lc_dc_query_attempts) {
  EXPECT_CALL(*_mock, zppValid(_, _)).WillOnce(Return(true));
  EXPECT_CALL(*_mock, ackbit(_)).Times(Exactly(0));
  auto validation_packet{PacketBuilder::makeZppValidQueryPacket("SP", 0uz)};
  Receive(validation_packet.timingsWithoutAckreq());
  Execute();
  Receive(validation_packet.timingsAckreqOnly());
  ASSERT_TRUE(Mock::VerifyAndClearExpectations(_mock.get()));

  auto packet{PacketBuilder::makeZppLcDcQueryPacket(_developer_code)};
  for (auto attempt{0uz}; attempt < MDU_MAX_LC_DC_QUERY_ATTEMPTS + 3uz;
       ++attempt) {
    SCOPED_TRACE(attempt);
    auto const allowed{attempt < MDU_MAX_LC_DC_QUERY_ATTEMPTS};
    EXPECT_CALL(*_mock, loadCodeValid({_developer_code}))
      .Times(allowed ? 1 : 0)
      .WillRepeatedly(Return(true));
    EXPECT_CALL(*_mock, ackbit(100u)).Times(allowed ? 0 : 3);

    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(_mock.get()));
  }
}
