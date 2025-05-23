#include "zpp_test.hpp"

ReceiveZppTest::ReceiveZppTest() { _mock = std::make_unique<ZppMock>(_cfg); }
