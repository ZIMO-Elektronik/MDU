// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive base
///
/// \file   mdu/rx/base.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <functional>
#include <gsl/util>
#include <ztl/inplace_deque.hpp>
#include "../bit.hpp"
#include "../crc32.hpp"
#include "../crc8.hpp"
#include "binary_tree_search.hpp"
#include "config.hpp"
#include "mixin.hpp"

namespace mdu::rx::detail {

struct ZsuMixin;

/// Receive base
///
/// \tparam Ts... Types of mixins
template<Mixin... Ts>
struct Base : Ts... {
  /// Ctor
  ///
  /// \param  cfg Confiuration
  explicit constexpr Base(Config cfg) : _cfg{cfg} {}

  /// Ctor
  ///
  /// \param  cfg                 Confiuration
  /// \param  salsa20_master_key  Salsa20 master key
  explicit constexpr Base(Config cfg, char const* salsa20_master_key)
    : ZsuMixin{salsa20_master_key}, _cfg{cfg} {}

  /// Dtor
  virtual constexpr ~Base() = default;

  /// Encoding of commands bit by bit
  ///
  /// \param  time  Time in µs
  void receive(uint32_t time) {
    auto const bit{time2bit(time, _transfer_rate_index)};
    if (bit == Ackreq) _state = &Base::ackreq;  // Shortcut to ackreq phase
    std::invoke(_state, this, time, bit);
  }

  /// Get active status (MDU is active when at least one preamble was received)
  ///
  /// \return true  MDU active
  /// \return false MDU not active
  bool active() const { return _active; }

  /// Execute
  void execute() {
    if (empty(_deque)) return;
    auto const& packet{_deque.front()};
    auto const command{packet2command(packet)};

    // Pop deque on exit
    gsl::final_action pop_deque{[this] { _deque.pop_front(); }};

    // Ping must always work, even when not selected
    if (command == Command::Ping) return executePing(packet);

    if (!selected()) return;

    switch (command) {
      case Command::ConfigTransferRate: {
        auto const transfer_rate{static_cast<TransferRate>(packet[4uz])};
        return executeConfigTransferRate(transfer_rate);
      }
      case Command::BinaryTreeSearch: {
        uint32_t const pos{packet[4uz]};
        return executeBinaryTreeSearch(pos);
      }
      case Command::CvRead: [[fallthrough]];
      case Command::CvWrite: {
        auto const number{data2uint16(&packet[4uz])};
        assert(number > 0u);
        auto const addr{number - 1u};
        auto const value{packet[6uz]};
        return command == Command::CvRead ? executeCvRead(addr, value)
                                          : executeCvWrite(addr, value);
      }
      default: {
        auto const mixins_ack{
          (Ts::execute(command, packet, _cfg.decoder_id) || ...)};
        return ack(mixins_ack);
      }
    }
  }

protected:
  /// Generate current pulse of length "us" in µs
  ///
  /// \param  us
  virtual void ackbit(uint32_t us) const = 0;

  /// Read CV bit
  ///
  /// \param  addr  CV address
  /// \param  pos   Bit position to test
  /// \return true  Bit set
  /// \return false Bit clear
  virtual bool readCv(uint32_t addr, uint32_t pos) const = 0;

  /// Write CV
  ///
  /// \param  addr  CV address
  /// \param  value CV value
  /// \return true  Success
  /// \return false Failure
  virtual bool writeCv(uint32_t addr, uint8_t value) = 0;

  /// Wait for preamble
  ///
  /// \param  bit Bit
  void preamble(uint32_t, Bit bit) {
    // Preamble can only set nack, never clear it
    if (bit == 1u) nack(++_bit_count >= 2uz || nack());
    else if (_bit_count < MDU_RX_PREAMBLE_BITS) reset();
    else {
      _bit_count = 0uz;
      active(true);
      _state = &Base::data;
    }
  }

  /// Receive data
  ///
  /// \param  bit Bit
  void data(uint32_t, Bit bit) {
    if (bit > 1u) return reset();
    else if (!shiftIn(bit)) return;
    else _state = &Base::endbit;
  }

  /// Might be packet end
  ///
  /// \param  bit Bit
  void endbit(uint32_t, Bit bit) {
    if (!bit) {
      _state = &Base::data;
      return;
    } else if (bit != 1u) return reset();
    if (packetValid()) _deque.push_back();
    _bit_count = 0u;
    _state = &Base::ackreq;
  }

  /// Ackreq phase
  ///
  /// \param  time  Time in µs
  /// \param  bit   Bit
  void ackreq(uint32_t time, Bit bit) {
    if (!selected() || bit != Ackreq) return reset();
    if (++_ackreqbit_count < 2uz) return;
    // Channel1 (incomplete packages or CRC errors)
    if (auto const& us{is_fallback_ackreq(time)
                         ? fallback_timing
                         : timings[_transfer_rate_index]};
        _ackreqbit_count >= 2uz && _ackreqbit_count <= 4uz) {
      if (nack()) ackbit(us.ack);
    }
    // Channel2
    else if (_ackreqbit_count >= 6uz && _ackreqbit_count <= 8uz)
      if (ack()) ackbit(us.ack);
  }

  /// Shift bit in
  ///
  /// \param  bit   Bit
  /// \return true  Byte done
  /// \return false Byte not yet done
  bool shiftIn(uint32_t bit) {
    assert(bit <= 1u);
    _byte |= static_cast<decltype(_byte)>(bit << (7uz - _bit_count++));
    if (_bit_count >= 8uz) {
      _crc8.next(_byte);
      _crc32.next(_byte);
      end(_deque)->push_back(_byte);
      _bit_count = _byte = 0u;
    }
    return !_bit_count;
  }

  /// Reset
  void reset() {
    end(_deque)->resize(0uz);
    _bit_count = _ackreqbit_count = _byte = 0u;
    ack(false);
    _crc8.reset();
    _crc32.reset();
    _state = &Base::preamble;
  }

  /// Check busy and CRC
  ///
  /// \return true  Packet valid
  /// \return false Packet not valid
  bool packetValid() {
    auto const deque_almost_full{size(_deque) >= _deque.max_size() - 1uz};
    auto const command{packet2command(*cend(_deque))};
    return !busy(command, deque_almost_full) && crcCheck(command);
  }

  /// Check if busy, setup nack/ack transmission
  ///
  /// \param  cmd               Command
  /// \param  deque_almost_full Only 1 slot left in deque
  /// \return true              Busy
  /// \return false             Not busy
  bool busy(Command cmd, bool deque_almost_full) {
    if (cmd == Command::Busy) {
      nack(_crc8);
      if (!_crc8) ack(deque_almost_full);
    }
    return deque_almost_full;
  }

  /// Check if CRC is valid, setup nack/ack transmission
  ///
  /// \param  cmd   Command
  /// \return true  CRC is valid
  /// \return false CRC is not valid
  bool crcCheck(Command cmd) {
    uint32_t crc;
    // Commands with CRC32 also transmit failures in channel2
    if (cmd == Command::ZsuUpdate || cmd == Command::ZppUpdate) {
      crc = _crc32;
      ack(crc);
    } else {
      crc = _crc8;
      // And there's also an exception for ZsuSalsa20IV...
      if (cmd == Command::ZsuSalsa20IV) ack(crc);
    }
    nack(crc);
    return !crc;
  }

  /// Set active status
  ///
  /// \param  active  Active
  void active(bool active) { _active = active; }

  /// Get selected status
  ///
  /// \return true  Decoder selected
  /// \return false Decoder not selected
  bool selected() const { return _selected; }

  /// Set selected status
  ///
  /// \param  selected  Selected
  void select(bool selected) { _selected = selected; }

  /// Get nack status
  ///
  /// \return true  Transmit ackbit in channel1
  /// \return false Do not transmit ackbit in channel1
  bool nack() const { return _nack; }

  /// Set nack status
  ///
  /// \param  nack  Nack
  void nack(bool nack) { _nack = nack; }

  /// Get ack status
  ///
  /// \return true  Transmit ackbit in channel2
  /// \return false Do not transmit ackbit in channel2
  bool ack() const { return _ack; }

  /// Set ack status
  ///
  /// \param  ack Ack
  void ack(bool ack) { _ack = ack; }

  /// Execute ping (short and long version)
  ///
  /// \param  packet  Packet
  void executePing(Packet const& packet) {
    uint32_t serial_number{};
    uint32_t decoder_id{};
    if (size(packet) < 9uz)
      decoder_id = packet[4uz] ? static_cast<uint32_t>(packet[4uz] << 24u) |
                                   (_cfg.decoder_id & 0x00FF'FFFFu)
                               : 0u;
    else {
      serial_number = data2uint32(&packet[4uz]);
      if (size(packet) >= 12uz) decoder_id = data2uint32(&packet[8uz]);
    }
    executePing(serial_number, decoder_id);
  }

  /// Execute ping
  ///
  /// \param  serial_number Serial number
  /// \param  decoder_id    Decoder ID
  void executePing(uint32_t serial_number, uint32_t decoder_id) {
    // Set ack on exit
    gsl::final_action set_ack{[this] { ack(_selected); }};
    if (serial_number && decoder_id)
      select(serial_number == _cfg.serial_number &&
             decoder_id == _cfg.decoder_id);
    else if (serial_number) select(serial_number == _cfg.serial_number);
    else if (decoder_id) select(decoder_id == _cfg.decoder_id);
    else select(true);
  }

  /// Execute config transfer rate
  ///
  /// \param  transfer_rate Transfer rate
  void executeConfigTransferRate(TransferRate transfer_rate) {
    if (transfer_rate < _cfg.transfer_rate) ack(true);
    else if (auto const i{std::to_underlying(transfer_rate)}; i < size(timings))
      _transfer_rate_index = i;
  }

  /// Execute binary tree search
  ///
  /// \param  pos Bit position to test
  void executeBinaryTreeSearch(uint32_t pos) {
    bool const bit{
      _binary_tree_search(_cfg.serial_number, _cfg.decoder_id, pos)};
    ack(bit);
  }

  /// Execute CV read
  ///
  /// \param  addr  CV address
  /// \param  pos   Bit position to test
  void executeCvRead(uint32_t addr, uint32_t pos) {
    bool const bit{readCv(addr, pos)};
    ack(bit);
  }

  /// Execute CV write
  ///
  /// \param  addr  CV address
  /// \param  value CV value
  void executeCvWrite(uint32_t addr, uint8_t value) {
    bool const success{writeCv(addr, value)};
    ack(!success);
  }

  void (Base::*_state)(uint32_t, Bit){&Base::preamble};
  size_t _bit_count{};        ///< Count received bits
  size_t _ackreqbit_count{};  ///< Count received ackreqbits
  Config const _cfg{};
  Crc32 _crc32{};
  ztl::inplace_deque<Packet, 2uz> _deque{};
  Crc8 _crc8;
  uint8_t _transfer_rate_index{std::to_underlying(TransferRate::Default)};
  uint8_t _byte{};
  BinaryTreeSearch _binary_tree_search{};
  bool _selected : 1 {true};
  bool _active : 1 {};
  bool _nack : 1 {};
  bool _ack : 1 {};
};

}  // namespace mdu::rx::detail