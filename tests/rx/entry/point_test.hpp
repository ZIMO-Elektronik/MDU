#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>
#include <utility>
#include <vector>

struct PointTest : ::testing::Test {
protected:
  void verify(std::vector<std::pair<uint32_t, uint8_t>> sequence);

  static constexpr uint32_t _decoder_id{};

  MOCK_METHOD(void, firmwareEntry, ());
  MOCK_METHOD(void, zppEntry, ());

  mdu::rx::entry::Point _entry_point{
    {.decoder_id = _decoder_id,
     .firmware_entry = [this] { firmwareEntry(); },
     .zpp_entry = [this] { zppEntry(); }}};
};