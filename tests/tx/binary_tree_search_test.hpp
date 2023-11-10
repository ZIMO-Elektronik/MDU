#pragma once

#include <gtest/gtest.h>
#include <mdu/mdu.hpp>
#include <vector>

struct BinaryTreeSearchTest : ::testing::Test {
  BinaryTreeSearchTest();

  void OneWireSearch();

  bool Reset();
  bool ReadBit(uint32_t pos, bool bit);
  bool SetSearchDirection(uint32_t pos, bool bit);

  struct Device {
    uint64_t id{};
    mdu::rx::BinaryTreeSearch bin_tree_search{};
  };

  std::vector<Device> _devices;
  std::vector<uint64_t> _found_ids;
};