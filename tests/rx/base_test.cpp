#include "base_test.hpp"

ReceiveBaseTest::ReceiveBaseTest() { base_ = std::make_unique<BaseMock>(cfg_); }