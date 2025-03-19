#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>
#include "../packet_builder.hpp"
#include "crtp_test_base.hpp"
#include "mock.hpp"

namespace mdu::tx::test {

using ::testing::NiceMock;

struct TransmitBaseTest : mdu::tx::test::CrtpTestBase<TransmitBaseTest> {
  TransmitBaseTest() {}
  NiceMock<Mock> _mock;
};

} // namespace mdu::tx::test