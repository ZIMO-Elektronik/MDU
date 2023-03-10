#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>
#include <utility>
#include <vector>

struct PointTest : ::testing::Test {
protected:
  void verify(std::vector<std::pair<size_t, uint8_t>> sequence);
  void verify(size_t index, uint8_t value);

  static constexpr uint32_t decoder_id_{};

  MOCK_METHOD(void, firmwareEntry, ());
  MOCK_METHOD(void, zppEntry, ());

  mdu::rx::entry::Point entry_point_{
    {.decoder_id = decoder_id_,
     .firmware_entry = [this] { firmwareEntry(); },
     .zpp_entry = [this] { zppEntry(); }}};
};