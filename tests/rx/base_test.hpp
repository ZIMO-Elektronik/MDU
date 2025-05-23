#pragma once

#include <mdu/mdu.hpp>
#include "base_mock.hpp"
#include "crtp_test_base.hpp"

struct ReceiveBaseTest : CrtpTestBase<ReceiveBaseTest> {
  ReceiveBaseTest();
  std::unique_ptr<BaseMock> _mock;
};
