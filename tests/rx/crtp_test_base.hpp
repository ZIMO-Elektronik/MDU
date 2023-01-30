#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>
#include <mdu/mdu.hpp>

template<typename T>
struct CrtpTestBase : ::testing::Test {
  friend T;

  void Receive(std::vector<uint32_t> times) {
    std::ranges::for_each(
      times, [this](uint32_t time) { impl().base_->receive(time); });
  }

  void ReceiveAndExecute(std::vector<uint32_t> times) {
    std::ranges::for_each(times, [this](uint32_t time) {
      impl().base_->receive(time);
      impl().base_->execute();
    });
  }

  void Execute() { impl().base_->execute(); }

  static constexpr auto ackreqbit_count_{10u};
  static constexpr auto serial_number_{0x70411AFCu};
  static constexpr auto decoder_id_{0x06043200u};
  static constexpr mdu::rx::Config cfg_{.serial_number = serial_number_,
                                        .decoder_id = decoder_id_};

private:
  constexpr CrtpTestBase() = default;
  auto& impl() { return static_cast<T&>(*this); }
  auto const& impl() const { return static_cast<T const&>(*this); }
};