#pragma once

#include "crtp_test_base.hpp"
#include "zpp_mock.hpp"

struct ReceiveZppTest : CrtpTestBase<ReceiveZppTest> {
  ReceiveZppTest();
  static constexpr std::array<uint8_t, 4uz> _developer_code{42u};
  std::unique_ptr<ZppMock> _mock;
};
