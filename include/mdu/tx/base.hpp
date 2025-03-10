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
#include "packet_type.hpp"
#include "state.hpp"
#include "typed_packet.hpp"

#include <span>
#include <utility>

namespace mdu::tx {

template<typename T>
struct CrtpBase {
  friend T;

  bool packet(TypedPacket const& packet) { return false; }
  bool bytes(std::span<uint8_t const> packet, PacketType type) { return false; }

  uint16_t transmit() {

    switch (_state) {
      case State::Preamble: return preambleTiming();
      case State::Packet: return packetTiming();
      case State::ACKreq: return ackreqTiming();
    }
  }

  /// Preamble timing
  ///
  /// \return preamble timing
  uint16_t preambleTiming() {
    _state = ++_bits < MDU_TX_MIN_PREAMBLE_BITS ? _state : State::Packet;
    if (_bits == MDU_TX_MIN_PREAMBLE_BITS) _bits = 0u;
    return timings[std::to_underlying(_rate)].one();
  }

  /// Packet timing
  ///
  /// \return next bit timing
  /// \todo DCC / MDU packets (MDU implemented)
  uint16_t packetTiming() {
    // Get next bit
    bool bit = (_packet.packet[_index] << _bits++) > 0;
    uint16_t timing = bit ? timings[std::to_underlying(_rate)].one()
                          : timings[std::to_underlying(_rate)].zero();

    // Check if byte transmit cplte
    if (_bits >= 8) {
      _bits = 0;
      _index++;
    }
    return timing;
  }

  /// ACKreq timing
  ///
  /// \return ackreq timing
  uint16_t ackreqTiming() { return 0; }

private:
  /// Packet
  TypedPacket _packet;

  /// Transfer rate
  TransferRate _rate;

  /// Transmit state
  State _state;

  /// Bits sent
  uint16_t _bits;

  /// Packet index
  uint16_t _index;
};

} // namespace mdu::tx
