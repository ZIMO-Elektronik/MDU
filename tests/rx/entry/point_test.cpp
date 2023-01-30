#include "point_test.hpp"
#include <algorithm>

void PointTest::verify(std::vector<std::pair<size_t, uint8_t>> sequence) {
  std::ranges::for_each(
    sequence, [this](auto p) { entry_point_.verify(p.first, p.second); });
}