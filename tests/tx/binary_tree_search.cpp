#include "binary_tree_search_test.hpp"

TEST_F(BinaryTreeSearchTest, find_randomly_generated_ids) {
  OneWireSearch();
  std::ranges::sort(_devices,
                    [](auto&& lhs, auto&& rhs) { return lhs.id < rhs.id; });
  std::ranges::sort(_found_ids);
  EXPECT_TRUE(
    std::ranges::equal(_devices, _found_ids, [](auto&& device, auto&& id) {
      return device.id == id;
    }));
}
