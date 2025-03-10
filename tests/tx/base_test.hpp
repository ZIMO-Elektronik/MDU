#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>
#include "crtp_test_base.hpp"
#include "mock.hpp"

namespace mdu::tx::test {

struct TransmitBaseTest : mdu::tx::test::CrtpTestBase<TransmitBaseTest> {
  TransmitBaseTest() { _mock = std::make_unique<Mock>(); }
  std::unique_ptr<Mock> _mock;
};

} // namespace mdu::tx::test