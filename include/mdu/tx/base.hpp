// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit base
///
/// \file   mdu/tx/base.hpp
/// \author Jonas Gahlert
/// \date   10.03.2025

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
#include "dcc/tx/track_outputs.hpp"
#include "phase.hpp"

#include <dcc/dcc.hpp>

namespace mdu::tx {

/// Transmit Base class
///
/// \details
/// Converts MDU packets to zero-cross times (in µs) "on-the-fly". Packets
/// can be transmitted using \ref Base::packet "packet". Alternatively, a
/// byte-span can be transmitted using \ref Base::bytes "bytes". A call to \ref
/// Base::transmit "transmit" yields the next timing. Currently, in the
/// Idle-phase an endlessly long preamble will be sent.
///
/// Configuration can be done using \ref Base::init "init" and the current
/// transfer rate can be adjusted using \ref Base::setTransferRate
/// "setTransferRate". Both can only be done if MDU is not busy.
///
/// \tparam T Inheriting class.
template<typename T>
struct Base {
  friend T;

  /// Initialize
  ///
  /// \param [in] cfg Configuration
  /// \retval true  Config set
  /// \retval false Config not set
  bool init(Config cfg = {}) {
    // Only init when idle
    if (_phase != Phase::Idle) return false;

    assert(cfg.num_preamble >= MDU_TX_MIN_PREAMBLE_BITS &&
           cfg.num_preamble <= MDU_TX_MAX_PREAMBLE_BITS &&
           cfg.num_ackreq >= MDU_TX_MIN_ACKREQ_BITS &&
           cfg.num_ackreq <= MDU_TX_MAX_ACKREQ_BITS);

    _cfg = cfg;
    return true;
  }

  /// Set transfer rate
  ///
  /// \param tRate  Transfer rate
  /// \retval true  Transfer rate set
  /// \retval false Transfer rate not set
  bool setTransferRate(TransferRate tRate) {
    // Only adjust transfer rate when idle
    if (_phase != Phase::Idle) return false;

    _rate = tRate;
    return true;
  }

  /// Set packet to transmit, transmitter must be idle.
  ///
  /// \param [in] packet  Packet to transmit
  /// \retval             true  Packet was enqueued
  /// \retval             false Transmission ongoing
  bool packet(Packet const& packet) {
    if (_phase != Phase::Idle) return false;

    _packet = packet;
    _phase = Phase::Preamble;
    return true;
  }

  /// Set bytes to transmit, transmitter must be idle.
  ///
  /// \param [in] bytes Bytes to transmit
  /// \param [in] type  Packet type
  /// \retval           true  Packet was enqueued
  /// \retval           false Transmission ongoing
  bool bytes(std::span<uint8_t const> bytes) {
    if (_phase != Phase::Idle) return false;

    // Copy bytes into package
    _packet.clear();
    std::copy_n(bytes.begin(), bytes.size(), std::back_inserter(_packet));
    _phase = Phase::Preamble;
    return true;
  }

  /// Convert next bit to timing (in µs)
  ///
  /// \return Converted timing
  uint16_t transmit() {

    switchTrack();
    switch (_phase) {
      case Phase::Idle: return idleTiming();
      case Phase::Preamble: return preambleTiming();
      case Phase::Packet: return packetTiming();
      case Phase::PacketEnd: return packetEndTiming();
      case Phase::ACKreq: return ackreqTiming();
      // Undefined
      default: return 0u;
    }
  }

private:
  constexpr Base() = default;
  CommandStation auto& impl() { return static_cast<T&>(*this); }
  CommandStation auto const& impl() const {
    return static_cast<T const&>(*this);
  }

  /// Idle timing
  ///
  /// \return idle timing
  uint16_t idleTiming() {
    // Send preamble of same type as last packet
    uint16_t timing = timings[std::to_underlying(_rate)].one;

    _bits++;
    return timing;
  }

  /// MDU Preamble timing
  ///
  /// \return preamble timing
  uint16_t preambleTiming() {
    uint16_t timing = timings[std::to_underlying(_rate)].one;
    _bits++;

    // Check if preamble cplte
    if (_bits >= _cfg.num_preamble && !_pol) {
      // Set initial state for packet transmit
      _bits = 8;
      _phase = Phase::Packet;
    }
    return timing;
  }

  /// MDU Packet timing
  ///
  /// \return next bit timing
  uint16_t packetTiming() {
    /*  _bits counts from 8 down to 0:
     *  -- 8 is StartBit
     *  -- 7..0 are Byte MSB first
     *  -- -1 is reset condition at end
     */

    // Get next bit
    bool bit = (_packet[_index] & (1u << _bits--)) > 0;
    uint16_t timing = bit ? timings[std::to_underlying(_rate)].one
                          : timings[std::to_underlying(_rate)].zero;

    // Check if byte transmit cplte
    if (_bits < 0) {
      // Byte cplte
      if (++_index >= _packet.size()) {
        // Packet cplte -> set initial state for ackreq
        _phase = Phase::PacketEnd;
        _bits = 0;
      } else {
        // Reset bit counter to StartBit
        _bits = 8;
      }
    }
    return timing;
  }

  /// Packet end timing
  ///
  /// \return packet end timing
  uint16_t packetEndTiming() {
    uint16_t timing = timings[std::to_underlying(_rate)].one;
    _phase = Phase::ACKreq;
    return timing;
  }

  /// MDU ACKreq timing
  ///
  /// \return ackreq timing
  uint16_t ackreqTiming() {
    uint16_t timing = timings[std::to_underlying(_rate)].ackreq;

    if (_bits == 0) impl().ackreqBegin();

    if (_bits >= 2 && _bits <= 4) impl().ackreqChannel1(_bits);
    if (_bits >= 6 && _bits <= 8) impl().ackreqChannel2(_bits);

    // Check for ACKreq end
    if (++_bits == _cfg.num_ackreq) {
      // Set initial state for ACKreq
      impl().ackreqEnd();
      _bits = _index = 0u;
      _phase = Phase::Idle;
    }
    return timing;
  }

  void switchTrack() {
    if constexpr (dcc::tx::TrackOutputs<T>) {
      !(_pol) ? impl().trackOutputs(false ^ _cfg.invert, true ^ _cfg.invert)
              : impl().trackOutputs(true ^ _cfg.invert, false ^ _cfg.invert);

      _pol = !_pol;
    }
  }

  /// Packet
  Packet _packet{};

  /// Transfer rate
  TransferRate _rate{TransferRate::Default};

  /// Transmit state
  Phase _phase{Phase::Idle};

  /// Bits counter
  int _bits{};

  /// Current track polarity
  bool _pol{false};

  /// Packet index
  uint16_t _index{};

  /// Config
  Config _cfg{};
};

} // namespace mdu::tx
