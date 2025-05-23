#include "binary_tree_search_test.hpp"
#include <numeric>
#include <random>

namespace {

uint64_t random_interval(uint64_t min, uint64_t max) {
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<uint64_t> dis{min, max};
  return dis(gen);
}

} // namespace

// Generate a couple of IDs to find
BinaryTreeSearchTest::BinaryTreeSearchTest() {
  auto const n{random_interval(10uz, 20uz)};
  for (auto i{0uz}; i < n; ++i)
    _devices.push_back(
      Device{.id = random_interval(0xFFFFFFFFull, 0x7FFFFFFF'FFFFFFFFull)});
}

// Run binary tree search algorithm
// (https://www.analog.com/en/app-notes/1wire-search-algorithm.html)
void BinaryTreeSearchTest::OneWireSearch() {
  uint64_t id{};
  uint32_t last_discrepancy{};
  bool last_device{};
  bool search_direction{};

  while (!last_device) {
    // Reset
    Reset();
    uint32_t last_zero{};

    for (auto id_bit_number{1u}; id_bit_number < sizeof(uint64_t) * CHAR_BIT;
         ++id_bit_number) {
      auto const i{id_bit_number - 1u};

      // Read bit
      auto const id_bit{ReadBit(i, true)};

      // Read complement bit
      auto const cmp_id_bit{ReadBit(i, false)};

      // Bit and complement can't be same
      assert(id_bit || cmp_id_bit);

      if (id_bit != cmp_id_bit)
        // Set search_direction to id_bit
        search_direction = id_bit;

      // Discrepancy
      else {
        if (id_bit_number < last_discrepancy)
          // Set search_direction to id_bit_number in id
          search_direction = id & (1ull << i);
        else search_direction = id_bit_number == last_discrepancy;

        if (!search_direction)
          // Set last_zero to current id_bit_number
          last_zero = id_bit_number;
      }

      // Set id_bit_number in id to search_direction and send to 1-Wire
      auto const bit_mask{1ull << i};
      if (search_direction) id |= bit_mask;
      else id &= ~bit_mask;
      SetSearchDirection(i, search_direction);
    }

    // Found ID
    _found_ids.push_back(id);

    last_discrepancy = last_zero;

    if (!last_discrepancy) last_device = true;
  }
}

// Wrap binary tree search special value "255" (reset) into convenience function
bool BinaryTreeSearchTest::Reset() {
  return std::accumulate(
    begin(_devices), end(_devices), false, [](bool lhs, auto&& device) mutable {
      return lhs |
             device.bin_tree_search(static_cast<uint32_t>(device.id),
                                    static_cast<uint32_t>(device.id >> 32u),
                                    255u);
    });
}

// Wrap binary tree search values to read bits into convenience function
bool BinaryTreeSearchTest::ReadBit(uint32_t pos, bool bit) {
  return std::accumulate(begin(_devices),
                         end(_devices),
                         false,
                         [&](bool lhs, auto&& device) mutable {
                           return lhs |
                                  device.bin_tree_search(
                                    static_cast<uint32_t>(device.id),
                                    static_cast<uint32_t>(device.id >> 32u),
                                    pos + (bit ? 0u : 64u));
                         });
}

// Wrap binary tree search values to set search direction into convenience
// function
bool BinaryTreeSearchTest::SetSearchDirection(uint32_t pos, bool bit) {
  return std::accumulate(begin(_devices),
                         end(_devices),
                         false,
                         [&](bool lhs, auto&& device) mutable {
                           return lhs |
                                  device.bin_tree_search(
                                    static_cast<uint32_t>(device.id),
                                    static_cast<uint32_t>(device.id >> 32u),
                                    pos + (!bit ? 128u : 192u));
                         });
}
