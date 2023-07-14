#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>

struct BaseMock : mdu::rx::detail::Base<> {
  using mdu::rx::detail::Base<>::Base;
  using mdu::rx::detail::Base<>::shiftIn;
  using mdu::rx::detail::Base<>::selected;
  using mdu::rx::detail::Base<>::select;
  using mdu::rx::detail::Base<>::_deque;
  MOCK_METHOD(void, ackbit, (uint32_t), (const, override));
  MOCK_METHOD(bool, readCv, (uint32_t, uint32_t), (const, override));
  MOCK_METHOD(bool, writeCv, (uint32_t, uint8_t), (override));
};