#include "firmware_test.hpp"

ReceiveFirmwareTest::ReceiveFirmwareTest() {
  base_ =
    std::make_unique<FirmwareMock>(cfg_, "cw4i14l79RPdQOJejv5xVMJPQ1UVnCHw");
}