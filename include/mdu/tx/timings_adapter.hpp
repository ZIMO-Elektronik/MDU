// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Timings iterator/view which converta packet to timings
///
/// \file   mdu/tx/timings_adapter.hpp
/// \author Jonas Gahlert
/// \date   10.06.2025

#pragma once

#include <cstdint>
#include <iterator>
#include <ranges>
#include <span>
#include "timings.hpp"

namespace mdu::tx {

/// TimingsAdapter
class TimingsAdapter : public std::ranges::view_interface<TimingsAdapter> {
  struct Sentinel {};

public:
  using value_type = Timings::value_type;
  using size_type = Timings::size_type;
  using difference_type = Timings::difference_type;
  using reference = value_type;
  using pointer = value_type;
  using iterator_category = std::input_iterator_tag;

  constexpr TimingsAdapter() = default;
  constexpr TimingsAdapter(Packet const& packet,
                           Config cfg,
                           TransferRate transfer_rate)
    : _packet{packet}, _cfg{cfg}, _transfer_rate{transfer_rate},
      _max_count{static_cast<size_type>(
        _cfg.num_preamble + std::size(_packet) * (1uz + CHAR_BIT) + 1uz)} {}
  constexpr TimingsAdapter(std::span<uint8_t const> bytes,
                           Config cfg,
                           TransferRate transfer_rate)
    : _cfg{cfg}, _transfer_rate{transfer_rate},
      _max_count{static_cast<size_type>(
        _cfg.num_preamble + std::size(bytes) * (1uz + CHAR_BIT) + 1uz)} {
    std::ranges::copy(bytes, std::back_inserter(_packet));
  }

  constexpr TimingsAdapter& operator++() {
    ++_count;
    return *this;
  }

  constexpr TimingsAdapter operator++(int) {
    auto retval{*this};
    ++(*this);
    return retval;
  }

  constexpr reference operator*() const {
    // Preamble
    auto const preamble_count{_cfg.num_preamble};
    if (_count < preamble_count)
      return timings[std::to_underlying(_transfer_rate)].one;

    // Count without preamble
    auto const i{static_cast<size_t>(_count - preamble_count)};

    // Index of current byte
    auto const byte_index{
      static_cast<Packet::size_type>(i / ((1uz + CHAR_BIT)))};
    if (byte_index >= std::size(_packet))
      return timings[std::to_underlying(_transfer_rate)].one;

    // Index of current bit
    auto const bit_index{i % (1uz + CHAR_BIT)};
    if (bit_index == 0uz)
      return timings[std::to_underlying(_transfer_rate)].zero;
    return _packet[byte_index] & 1u << (CHAR_BIT - 1uz - (bit_index - 1uz))
             ? timings[std::to_underlying(_transfer_rate)].one
             : timings[std::to_underlying(_transfer_rate)].zero;
  }

  constexpr bool operator==(Sentinel) const { return _count >= _max_count; }

  constexpr TimingsAdapter& begin() { return *this; }
  constexpr TimingsAdapter const& begin() const { return *this; }
  constexpr Sentinel end() { return {}; }
  constexpr Sentinel end() const { return {}; }
  constexpr Sentinel cend() { return {}; }
  constexpr Sentinel cend() const { return {}; }

private:
  Packet _packet;                ///< MDU packet
  Config _cfg{};                 ///< Config
  TransferRate _transfer_rate{}; ///< Transfer rate
  size_type _count{};            ///< Bit counter
  size_type _max_count{};        ///< Max bit counter value
};

constexpr auto begin(TimingsAdapter& c) -> decltype(c.begin()) {
  return c.begin();
}
constexpr auto begin(TimingsAdapter const& c) -> decltype(c.begin()) {
  return c.begin();
}
constexpr auto end(TimingsAdapter& c) -> decltype(c.end()) { return c.end(); }
constexpr auto end(TimingsAdapter const& c) -> decltype(c.end()) {
  return c.end();
}
constexpr auto cbegin(TimingsAdapter const& c) -> decltype(c.begin()) {
  return c.begin();
}
constexpr auto cend(TimingsAdapter const& c) -> decltype(c.end()) {
  return c.end();
}

} // namespace mdu::tx
