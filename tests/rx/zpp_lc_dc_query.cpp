#include "utility.hpp"
#include "zpp_test.hpp"

using namespace ::testing;

namespace std {

// Need operator overload for EXPECT_CALL
bool operator==(span<uint8_t const, 4uz> lhs, span<uint8_t const, 4uz> rhs) {
  return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
}

}  // namespace std

TEST_F(ReceiveZppTest, loadcode_valid) {
  {
    Expectation validate_zpp{EXPECT_CALL(*base_, zppValid(_, _))
                               .Times(Exactly(1))
                               .WillRepeatedly(Return(true))};
    auto packet{make_zpp_valid_query_packet("SP", 0uz)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }

  {
    Expectation loadcode_valid{
      EXPECT_CALL(*base_, loadcodeValid({developer_code_}))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true))};
    auto packet{make_zpp_lc_dc_query_packet(developer_code_)};
    Receive(packet.timingsWithoutAckreq());
    Execute();
    Receive(packet.timingsAckreqOnly());
  }
}