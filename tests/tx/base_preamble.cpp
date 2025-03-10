#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, send_preamble) {
  ASSERT_EQ(Transmit(),
            mdu::timings[std::to_underlying(TransferRate::Default)].one);
}

} // namespace mdu::tx::test