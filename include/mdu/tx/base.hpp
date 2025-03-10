// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit base
///
/// \file   mdu/tx/crtp_base.hpp
/// \author Jonas Gahlert
/// \date   10.03.2025

#pragma once

#include "../timing.hpp"
#include "../transfer_rate.hpp"
#include "command_station.hpp"
#include "packet_type.hpp"
#include "state.hpp"
#include "typed_packet.hpp"

#include <algorithm>
#include <span>
#include <utility>

namespace mdu::tx {

template<typename T>
struct Base {
  friend T;

  bool packet(TypedPacket const& packet) {
    if (_state != State::Idle) return false;
    _state = State::Preamble;
    _packet = packet;
    return true;
  }

  bool bytes(std::span<uint8_t const> bytes, PacketType type) {
    if (_state != State::Idle) return false;
    _state = State::Preamble;

    // Copy bytes into package
    _packet.packet.clear();
    std::copy_n(
      bytes.begin(), bytes.size(), std::back_inserter(_packet.packet));
    _packet.type = type;
    return true;
  }

  uint16_t transmit() {

    switch (_state) {
      case State::Idle: return preambleTiming(true);
      case State::Preamble: return preambleTiming(false);
      case State::Packet: return packetTiming();
      case State::ACKreq: return ackreqTiming();
      default: return 0u;
    }
  }

private:
  constexpr Base() = default;
  CommandStation auto& impl() { return static_cast<T&>(*this); }
  CommandStation auto const& impl() const {
    return static_cast<T const&>(*this);
  }

  /// Preamble timing
  ///
  /// \return preamble timing
  uint16_t preambleTiming(bool idle) {
    uint16_t timing = timings[std::to_underlying(_rate)].one;
    _bits++;

    // return here if were in idle state
    if (idle) return timing;

    // Check if preamble cplte
    if (_bits >= MDU_TX_MIN_PREAMBLE_BITS) {
      _bits = 0u;
      _state = State::Packet;
    }
    return timing;
  }

  /// Packet timing
  ///
  /// \return next bit timing
  /// \todo DCC / MDU packets (MDU implemented)
  uint16_t packetTiming() {
    // Get next bit
    bool bit = (_packet.packet[_index] << _bits++) > 0;
    uint16_t timing = bit ? timings[std::to_underlying(_rate)].one
                          : timings[std::to_underlying(_rate)].zero;

    // Check if byte transmit cplte
    if (_bits >= 8) {
      _bits = 0;
      // Check if packet transmit cplte
      _state = ++_index < _packet.packet.size() ? _state : State::ACKreq;
    }
    return timing;
  }

  /// ACKreq timing
  ///
  /// \return ackreq timing
  uint16_t ackreqTiming() {
    _state = ++_bits < MDU_TX_MIN_ACKREQ_BITS ? _state : State::Preamble;
    if (_bits == MDU_TX_MIN_ACKREQ_BITS) _bits = 0u;
    uint16_t timing = timings[std::to_underlying(_rate)].ackreq;
    return timing;
  }

private:
  /// Packet
  TypedPacket _packet{};

  /// Transfer rate
  TransferRate _rate{TransferRate::Default};

  /// Transmit state
  State _state{State::Idle};

  /// Bits sent
  uint16_t _bits{};

  /// Packet index
  uint16_t _index{};
};

} // namespace mdu::tx
