#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>
#include <mdu/mdu.hpp>

template<typename T>
struct CrtpTestBase : ::testing::Test {
  friend T;

  void Receive(mdu::tx::Timings timings) {
    std::ranges::for_each(timings,
                          [this](uint32_t t) { impl()._mock->receive(t); });
  }

  void ReceiveAndExecute(mdu::tx::Timings timings) {
    std::ranges::for_each(timings, [this](uint32_t t) {
      impl()._mock->receive(t);
      impl()._mock->execute();
    });
  }

  void Execute() { impl()._mock->execute(); }

  static constexpr uint32_t _serial_number{0x70411AFCu};
  static constexpr uint32_t _decoder_id{0x06043200u};
  static constexpr mdu::rx::Config _cfg{.serial_number = _serial_number,
                                        .decoder_id = _decoder_id};

private:
  constexpr CrtpTestBase() = default;
  auto& impl() { return static_cast<T&>(*this); }
  auto const& impl() const { return static_cast<T const&>(*this); }
};
