#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>
#include "../packet_builder.hpp"
#include "mock.hpp"

using ::testing::NiceMock;

struct TransmitBaseTest : ::testing::Test {
protected:
  TransmitBaseTest() {}

  mdu::tx::Timings Transmit(size_t count) {
    mdu::tx::Timings retval{};
    for (auto i{0uz}; i < count; i++) retval.push_back(_mock.transmit());
    return retval;
  }

  mdu::tx::Timings Transmit(mdu::Packet packet) {
    _mock.bytes(packet);

    mdu::tx::Timings retval{};
    // Preamble
    for (auto i{0uz}; i < _cfg.num_preamble; i++)
      retval.push_back(_mock.transmit());

    // Data
    for ([[maybe_unused]] auto it : packet) {
      retval.push_back(_mock.transmit());
      for (size_t i{CHAR_BIT}; i-- > 0uz;) retval.push_back(_mock.transmit());
    }

    // Stop Bit
    retval.push_back(_mock.transmit());

    // AckReq
    for (auto i{0uz}; i < _cfg.num_ackreq; i++)
      retval.push_back(_mock.transmit());

    return retval;
  }

  NiceMock<Mock> _mock;
  mdu::tx::Config _cfg{};
};
