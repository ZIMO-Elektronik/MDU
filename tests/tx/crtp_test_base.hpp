#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <array>
#include <mdu/mdu.hpp>

namespace mdu::tx::test {

template<typename T>
struct CrtpTestBase : ::testing::Test {
  friend T;

  void Enqueue4ByteDummy() {
    impl()._mock->bytes({std::array<uint8_t, 4>{1u, 2u, 3u, 4u}});
  }

  uint16_t Transmit() { return impl()._mock->transmit(); }

  void SkipPreamble() {
    for (int i = 0; i < MDU_TX_MIN_PREAMBLE_BITS; i++) impl()._mock->transmit();
  }

  std::array<uint16_t, 9> TransmitByte() {
    std::array<uint16_t, 9> ret{};
    for (auto& it : ret) { it = Transmit(); }
    return ret;
  }

  std::array<std::array<uint16_t, 9>, 4> Transmit4Bytes() {
    std::array<std::array<uint16_t, 9>, 4> ret{};
    for (auto& it : ret) { it = TransmitByte(); }
    return ret;
  }

  std::array<uint16_t, 10> TransmitACKreq() {
    std::array<uint16_t, 10> ret{};
    for (auto& it : ret) { it = Transmit(); }
    return ret;
  }

  std::array<uint16_t, 9> Byte2Timings(uint8_t byte) {
    std::array<uint16_t, 9> ret{};
    size_t cnt = 8u;
    for (auto& it : ret) {
      it = ((byte | 1) << cnt--) > 0
             ? mdu::timings[std::to_underlying(TransferRate::Default)].one
             : mdu::timings[std::to_underlying(TransferRate::Default)].zero;
    }
    return ret;
  }
  uint8_t Timings2Byte(std::array<uint16_t, 9> timings) {
    uint8_t ret = 0;
    size_t cnt = 8u;
    for (auto it : timings) {
      if (it == mdu::timings[std::to_underlying(TransferRate::Default)].one) {
        ret |= 1u << cnt;
      }
      cnt--;
    }
    return ret;
  }

private:
  constexpr CrtpTestBase() = default;
  auto& impl() { return static_cast<T&>(*this); }
  auto const& impl() const { return static_cast<T const&>(*this); }
};

} // namespace mdu::tx::test