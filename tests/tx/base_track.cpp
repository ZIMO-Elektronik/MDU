#include "base_test.hpp"

namespace mdu::tx::test {

TEST_F(TransmitBaseTest, track_outputs) {
  EXPECT_CALL(_mock, trackOutputs(true, false)).Times(3);
  EXPECT_CALL(_mock, trackOutputs(false, true)).Times(3);

  EXPECT_CALL(_mock, trackOutputs(true, true)).Times(0);
  EXPECT_CALL(_mock, trackOutputs(false, false)).Times(0);

  Transmit(6);
}

} // namespace mdu::tx::test