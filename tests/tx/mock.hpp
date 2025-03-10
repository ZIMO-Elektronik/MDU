#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>

namespace mdu::tx::test {

struct Mock : mdu::tx::Base<Mock> {
  MOCK_METHOD(void, ackreqStart, ());
  MOCK_METHOD(void, ackreqNext, ());
  MOCK_METHOD(void, ackreqEnd, ());
};

} // namespace mdu::tx::test