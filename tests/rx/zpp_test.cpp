#include "zpp_test.hpp"

ReceiveZppTest::ReceiveZppTest() { base_ = std::make_unique<ZppMock>(cfg_); }