#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>

struct BaseMock : mdu::rx::Base<> {
  using mdu::rx::Base<>::Base;
  using mdu::rx::Base<>::shiftIn;
  using mdu::rx::Base<>::selected;
  using mdu::rx::Base<>::select;
  using mdu::rx::Base<>::_deque;
  MOCK_METHOD(void, ackbit, (uint32_t), (const, override));
  MOCK_METHOD(bool, readCv, (uint32_t, uint32_t), (const, override));
  MOCK_METHOD(bool, writeCv, (uint32_t, uint8_t), (override));
};
