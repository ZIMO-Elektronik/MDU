#include "firmware_test.hpp"

ReceiveFirmwareTest::ReceiveFirmwareTest() {
  _mock =
    std::make_unique<FirmwareMock>(_cfg, "cw4i14l79RPdQOJejv5xVMJPQ1UVnCHw");
}