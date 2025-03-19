#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/tx/base.hpp>

namespace mdu::tx::test {

struct Mock : mdu::tx::Base<Mock> {
  MOCK_METHOD(void, ackreqBegin, ());
  MOCK_METHOD(void, ackreqChannel1, (int));
  MOCK_METHOD(void, ackreqChannel2, (int));
  MOCK_METHOD(void, ackreqEnd, ());
  MOCK_METHOD(void, trackOutputs, (bool, bool));
};

} // namespace mdu::tx::test