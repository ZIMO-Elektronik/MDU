#include "base_test.hpp"

ReceiveBaseTest::ReceiveBaseTest() { _mock = std::make_unique<BaseMock>(_cfg); }