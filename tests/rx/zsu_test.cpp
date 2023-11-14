#include "zsu_test.hpp"

ReceiveZsuTest::ReceiveZsuTest() {
  _mock = std::make_unique<ZsuMock>(_cfg, "cw4i14l79RPdQOJejv5xVMJPQ1UVnCHw");
}