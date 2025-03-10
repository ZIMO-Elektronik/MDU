#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mdu/mdu.hpp>

namespace mdu::tx::test {

template<typename T>
struct CrtpTestBase : ::testing::Test {
  friend T;

  uint16_t Transmit() { return impl()._mock->transmit(); }

private:
  constexpr CrtpTestBase() = default;
  auto& impl() { return static_cast<T&>(*this); }
  auto const& impl() const { return static_cast<T const&>(*this); }
};

} // namespace mdu::tx::test