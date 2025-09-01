// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit base
///
/// \file   mdu/tx/base.hpp
/// \author Jonas Gahlert
/// \date   10/03/2025

#pragma once

#include <algorithm>
#include <span>
#include <utility>
#include "../packet.hpp"
#include "../timing.hpp"
#include "../transfer_rate.hpp"
#include "channel.hpp"
#include "command_station.hpp"
#include "config.hpp"
#include "timings.hpp"
#include "timings_adapter.hpp"

namespace mdu::tx {

/// Transmit base
///
/// Converts MDU packets to zero-cross times (in µs). Packets can be transmitted
/// using Base::packet(). Alternatively, a byte-span can be transmitted using
/// Base::bytes(). A call to Base::transmit() yields the next timing. Currently,
/// in the idle phase an endlessly long preamble will be sent.
///
/// Configuration can be done using Base::init() and the current transfer rate
/// can be adjusted using Base::setTransferRate(). Both can only be done if MDU
/// is not busy.
///
/// \tparam T Type to downcast to
/// \tparam D Deque value type
/// \arg      Packet  Calculate on-the-fly
/// \arg      Timings Pre-calculate timings
template<typename T, typename D = Packet>
requires(std::same_as<D, Packet> || std::same_as<D, Timings>)
struct Base {
  friend T;

  using value_type =
    std::conditional_t<std::same_as<D, Packet>, TimingsAdapter, Timings>;

  /// Initialize
  ///
  /// \param  cfg   Configuration
  /// \retval true  Config set
  /// \retval false Busy
  bool init(Config cfg = {}) {
    // Only init when idle
    if (!isIdle()) return false;

    assert(cfg.num_preamble >= MDU_TX_MIN_PREAMBLE_BITS &&
           cfg.num_preamble <= MDU_TX_MAX_PREAMBLE_BITS &&
           cfg.num_ackreq >= MDU_TX_MIN_ACKREQ_BITS &&
           cfg.num_ackreq <= MDU_TX_MAX_ACKREQ_BITS);

    // This needs to be updated before cfg is saved
    _ackreq_count = cfg.num_ackreq;

    _cfg = cfg;
    return true;
  }

  /// Set transfer rate
  ///
  /// \param  transfer_rate Transfer rate
  /// \retval true          Transfer rate set
  /// \retval false         Busy
  bool setTransferRate(TransferRate transfer_rate) {
    // Only adjust transfer rate when idle
    if (!isIdle()) return false;

    _transfer_rate = transfer_rate;
    return true;
  }

  /// Is idle
  ///
  /// \retval true  Idle
  /// \retval false Busy
  bool isIdle() { return _iter == _last && _ackreq_count == _cfg.num_ackreq; }

  /// Set packet to transmit, transmitter must be idle
  ///
  /// \param  packet  Packet to transmit
  /// \retval true    Packet was enqueued
  /// \retval false   Transmission ongoing
  bool packet(Packet const& packet) {
    return bytes({cbegin(packet), std::size(packet)});
  }

  /// Set bytes to transmit, transmitter must be idle
  ///
  /// \param  bytes Bytes to transmit
  /// \retval true  Packet was enqueued
  /// \retval false Transmission ongoing
  bool bytes(std::span<uint8_t const> bytes) {
    // Only set packet when idle
    if (!isIdle()) return false;
    assert(std::size(bytes) <= MDU_MAX_PACKET_SIZE);

    // Copy bytes into package
    if constexpr (std::same_as<D, Packet>)
      _packet = {bytes, _cfg, _transfer_rate};
    else {
      auto tmp{
        bytes2timings({std::begin(bytes), std::size(bytes)},
                      {.num_preamble = _cfg.num_preamble, .num_ackreq = 0u},
                      _transfer_rate)};
      _packet.clear();
      std::ranges::copy_n(
        std::begin(tmp), std::size(tmp), std::back_inserter(_packet));
    }

    // Set iterators and reset ackreq counter
    _iter = std::begin(_packet);
    _last = std::cend(_packet);
    _ackreq_count = 0uz;
    return true;
  }

  /// Get next bit duration to transmit in µs
  ///
  /// \return Bit duration in µs
  Timings::value_type transmit() {
    toggleTrackOutputs();
    if (_iter != _last) return packetTiming();
    else if (_ackreq_count < _cfg.num_ackreq) return ackreqTiming();
    else return timings[std::to_underlying(_transfer_rate)].one;
  }

private:
  constexpr Base() = default;
  CommandStation auto& impl() { return static_cast<T&>(*this); }
  CommandStation auto const& impl() const {
    return static_cast<T const&>(*this);
  }

  /// Packet timing
  ///
  /// \return Next bit timing
  Timings::value_type packetTiming() {
    auto const retval{*_iter};
    ++_iter;
    return retval;
  }

  /// ACKreq timing
  ///
  /// \return Ackreq timing
  Timings::value_type ackreqTiming() {
    if (!_ackreq_count) impl().ackreqBegin();
    else if (detail::is_channel1(_ackreq_count))
      impl().ackreqChannel1(_ackreq_count);
    else if (detail::is_channel2(_ackreq_count))
      impl().ackreqChannel2(_ackreq_count);

    // Check for ACKreq end
    if (++_ackreq_count == _cfg.num_ackreq)
      impl().ackreqEnd(); // Set initial state for ACKreq

    return timings[std::to_underlying(_transfer_rate)].ackreq;
  }

  /// Toggle track output
  void toggleTrackOutputs() {
    if constexpr (requires(T t, bool N, bool P) {
                    { t.trackOutputs(N, P) };
                  }) {
      // By default the phase is "positive", so P > N for the first half bit.
      impl().trackOutputs(_polarity, !_polarity);
      _polarity = !_polarity;
    }
  }

  value_type _packet{}; ///< Packet

  /// Iterators
  decltype(std::begin(_packet)) _iter{std::begin(_packet)};
  decltype(std::cend(_packet)) _last{std::cend(_packet)};

  size_t _ackreq_count{MDU_TX_MIN_ACKREQ_BITS};       ///< ACKReq counter
  TransferRate _transfer_rate{TransferRate::Default}; ///< Transfer rate
  Config _cfg{};                                      ///< Config
  bool _polarity{}; ///< Current track polarity
};

} // namespace mdu::tx
