#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>
#include <utility>
#include <vector>

struct PointTest : ::testing::Test {
protected:
  void verify(std::vector<std::pair<uint32_t, uint8_t>> sequence);

  static constexpr uint32_t _decoder_id{0xCC00FFEEu};

  MOCK_METHOD(void, zppEntry, ());
  MOCK_METHOD(void, zsuEntry, ());

  mdu::rx::entry::Point _entry_point{{.decoder_id = _decoder_id,
                                      .zpp_entry = [this] { zppEntry(); },
                                      .zsu_entry = [this] { zsuEntry(); }}};
};