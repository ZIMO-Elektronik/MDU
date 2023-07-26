#pragma once

#include "crtp_test_base.hpp"
#include "firmware_mock.hpp"

struct ReceiveFirmwareTest : CrtpTestBase<ReceiveFirmwareTest> {
  ReceiveFirmwareTest();
  std::unique_ptr<FirmwareMock> _mock;
};