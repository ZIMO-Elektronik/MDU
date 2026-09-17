#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>

struct Mock : mdu::tx::PacketsBase {
  MOCK_METHOD(void, ackreqBegin, ());
  MOCK_METHOD(void, ackreqChannel1, (size_t));
  MOCK_METHOD(void, ackreqChannel2, (size_t));
  MOCK_METHOD(void, ackreqEnd, ());
  MOCK_METHOD(void, trackOutputs, (bool, bool));
};
