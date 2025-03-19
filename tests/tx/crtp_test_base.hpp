#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <array>
#include <mdu/mdu.hpp>

#include <algorithm>

namespace mdu::tx::test {

using Timings = std::vector<uint32_t>;
using Packet = std::vector<uint8_t>;

template<typename T>
struct CrtpTestBase : ::testing::Test {
  friend T;

  bool EvalPacket(Timings const& timings,
                  Packet const& packet,
                  Timings::size_type& offset,
                  TransferRate t_rate = TransferRate::Default,
                  Config cfg = {}) {
    if (!EvalPreamble(timings, offset, t_rate, cfg)) return false;
    for (auto it : packet) {
      if (!EvalStartBit(timings, offset, t_rate, cfg)) return false;
      if (!EvalByte(timings, it, offset, t_rate, cfg)) return false;
    }
    if (!EvalStopBit(timings, offset, t_rate, cfg)) return false;
    if (!EvalAckReq(timings, offset, t_rate, cfg)) return false;
    return true;
  }

  bool EvalPreamble(Timings const& timings,
                    Timings::size_type& offset,
                    TransferRate t_rate = TransferRate::Default,
                    Config cfg = {}) {
    for (Timings::size_type i = 0; i < cfg.num_preamble; i++) {
      if (timings[i] != mdu::timings[std::to_underlying(t_rate)].one)
        return false;
      offset++;
    }
    return true;
  }

  bool EvalStartBit(Timings const& timings,
                    Timings::size_type& offset,
                    TransferRate t_rate = TransferRate::Default,
                    Config cfg = {}) {
    if (timings[offset] != mdu::timings[std::to_underlying(t_rate)].zero)
      return false;
    offset++;
    return true;
  }

  bool EvalByte(Timings const& timings,
                Packet::value_type const byte,
                Timings::size_type& offset,
                TransferRate t_rate = TransferRate::Default,
                Config cfg = {}) {
    Packet::value_type eval{0};
    for (int i = 7; i > -1; i--) {
      if ((timings[offset] != mdu::timings[std::to_underlying(t_rate)].one) &&
          (timings[offset] != mdu::timings[std::to_underlying(t_rate)].zero))
        return false;

      if (timings[offset++] == mdu::timings[std::to_underlying(t_rate)].one)
        eval |= 1u << i;
    }
    if (eval != byte) return false;
    return true;
  }

  bool EvalStopBit(Timings const& timings,
                   Timings::size_type& offset,
                   TransferRate t_rate = TransferRate::Default,
                   Config cfg = {}) {
    if (timings[offset] != mdu::timings[std::to_underlying(t_rate)].one)
      return false;
    offset++;
    return true;
  }

  bool EvalAckReq(Timings const& timings,
                  Timings::size_type& offset,
                  TransferRate t_rate = TransferRate::Default,
                  Config cfg = {}) {
    for (Timings::size_type i = 0; i < cfg.num_ackreq; i++) {
      if (timings[offset] != mdu::timings[std::to_underlying(t_rate)].ackreq)
        return false;
      offset++;
    }
    return true;
  }

  void Enqueue4ByteDummy() {
    impl()._mock.bytes({std::array<uint8_t, 4>{1u, 2u, 3u, 4u}});
  }

  Timings::value_type Transmit() { return impl()._mock.transmit(); }

  Timings Transmit(int count) {
    Timings retval{};
    for (int i = 0; i < count; i++) {
      retval.push_back(impl()._mock.transmit());
    }
    return retval;
  }

  Timings Transmit(Packet packet, Config cfg = {}) {
    impl()._mock.bytes(packet);

    Timings retval{};
    // Preamble
    for (int i = 0; i < cfg.num_preamble; i++) {
      retval.push_back(impl()._mock.transmit());
    }

    // Data
    for ([[maybe_unused]] auto it : packet) {
      retval.push_back(impl()._mock.transmit());
      for ([[maybe_unused]] int i = 7; i > -1; i--) {
        retval.push_back(impl()._mock.transmit());
      }
    }

    // Stop Bit
    retval.push_back(impl()._mock.transmit());

    // AckReq
    for (int i = 0; i < cfg.num_ackreq; i++) {
      retval.push_back(impl()._mock.transmit());
    }

    return retval;
  }

  bool Packet(Packet packet) { return impl()._mock.bytes(packet); }

  void Init(Config cfg = {}) { impl()._mock.init(cfg); }

private:
  constexpr CrtpTestBase() = default;
  auto& impl() { return static_cast<T&>(*this); }
  auto const& impl() const { return static_cast<T const&>(*this); }
};

} // namespace mdu::tx::test