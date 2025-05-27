#pragma once

#include "crtp_test_base.hpp"
#include "zsu_mock.hpp"

struct ReceiveZsuTest : CrtpTestBase<ReceiveZsuTest> {
  ReceiveZsuTest();
  std::unique_ptr<ZsuMock> _mock;
};
