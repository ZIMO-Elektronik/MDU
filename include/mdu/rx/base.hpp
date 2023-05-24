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
#include <ztl/circular_array.hpp>
#include "../bit.hpp"
#include "../crc32.hpp"
#include "../crc8.hpp"
#include "binary_search.hpp"
#include "config.hpp"
#include "mixin.hpp"

namespace mdu::rx::detail {

struct FirmwareMixin;

/// Receive base
///
/// \tparam Ts... Types of mixins
template<Mixin... Ts>
struct Base : Ts... {
  /// Ctor
  ///
  /// \param  cfg Confiuration
  explicit constexpr Base(Config cfg) : cfg_{cfg} {}

  /// Ctor
  ///
  /// \param  cfg                 Confiuration
  /// \param  salsa20_master_key  Salsa20 master key
  explicit constexpr Base(Config cfg, char const* salsa20_master_key)
    : FirmwareMixin{salsa20_master_key}, cfg_{cfg} {}

  /// Dtor
  virtual constexpr ~Base() = default;

  /// Encoding of commands bit by bit
  ///
  /// \param  time  Time in µs
  void receive(uint32_t time) {
    auto const bit{time2bit(time, transfer_rate_index_)};
    if (bit == Ackreq) fp_ = &Base::ackreq;  // Shortcut to ackreq phase
    std::invoke(fp_, this, time, bit);
  }

  /// Get active status (MDU is active when at least one preamble was received)
  ///
  /// \return true  MDU active
  /// \return false MDU not active
  bool active() const { return active_; }

  /// Execute
  void execute() {
    if (empty(queue_)) return;
    auto const& packet{queue_.front()};
    auto const command{packet2command(packet)};

    // Pop queue on exit
    gsl::final_action pop_queue{[this] { queue_.pop_front(); }};

    // Ping must always work, even when not selected
    if (command == Command::Ping) return executePing(packet);

    if (!selected()) return;

    switch (command) {
      case Command::ConfigTransferRate: {
        auto const transfer_rate{static_cast<TransferRate>(packet.data[4uz])};
        return executeConfigTransferRate(transfer_rate);
      }
      case Command::BinarySearch: {
        uint32_t const position{packet.data[4uz]};
        return executeBinarySearch(position);
      }
      case Command::CvRead: [[fallthrough]];
      case Command::CvWrite: {
        auto const number{data2uint16(&packet.data[4uz])};
        assert(number > 0u);
        auto const addr{number - 1u};
        auto const value{packet.data[6uz]};
        return command == Command::CvRead ? executeCvRead(addr, value)
                                          : executeCvWrite(addr, value);
      }
      default: {
        auto const mixins_ack{
          (Ts::execute(command, packet, cfg_.decoder_id) || ...)};
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
  /// \param  addr      CV address
  /// \param  position  Bit position to test
  /// \return true      Bit set
  /// \return false     Bit clear
  virtual bool readCv(uint32_t addr, uint32_t position) const = 0;

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
    if (bit == 1u) ++bit_count_;
    else if (bit_count_ < MDU_RX_PREAMBLE_BITS) reset();
    else {
      bit_count_ = 0uz;
      active_ = nack_ = true;
      fp_ = &Base::data;
    }
  }

  /// Receive data
  ///
  /// \param  bit Bit
  void data(uint32_t, Bit bit) {
    if (bit > 1u) return reset();
    else if (!shiftIn(bit)) return;
    else fp_ = &Base::endbit;
  }

  /// Might be packet end
  ///
  /// \param  bit Bit
  void endbit(uint32_t, Bit bit) {
    if (!bit) {
      fp_ = &Base::data;
      return;
    } else if (bit != 1u) return reset();
    if (packetValid()) queue_.push_back();
    bit_count_ = 0u;
    fp_ = &Base::ackreq;
  }

  /// Ackreq phase
  ///
  /// \param  time  Time in µs
  /// \param  bit   Bit
  void ackreq(uint32_t time, Bit bit) {
    if (!selected() || bit != Ackreq) return reset();
    if (++ackreqbit_count_ < 2uz) return;
    // Channel1 (incomplete packages or CRC errors)
    if (auto const& us{is_fallback_ackreq(time)
                         ? fallback_timing
                         : timings[transfer_rate_index_]};
        ackreqbit_count_ >= 2uz && ackreqbit_count_ <= 4uz) {
      if (nack()) ackbit(us.ack);
    }
    // Channel2
    else if (ackreqbit_count_ >= 6uz && ackreqbit_count_ <= 8uz)
      if (ack()) ackbit(us.ack);
  }

  /// Shift bit in
  ///
  /// \param  bit   Bit
  /// \return true  Byte done
  /// \return false Byte not yet done
  bool shiftIn(uint32_t bit) {
    assert(bit <= 1u);
    byte_ |= static_cast<decltype(byte_)>(bit << (7uz - bit_count_++));
    if (bit_count_ >= 8uz) {
      crc8_.next(byte_);
      crc32_.next(byte_);
      auto& [data, size]{*end(queue_)};
      data[size++] = byte_;
      bit_count_ = byte_ = 0u;
    }
    return !bit_count_;
  }

  /// Reset
  void reset() {
    bit_count_ = ackreqbit_count_ = end(queue_)->size = byte_ = 0u;
    nack_ = ack_ = false;
    crc8_.reset();
    crc32_.reset();
    fp_ = &Base::preamble;
  }

  /// Check busy and CRC
  ///
  /// \return true  Packet valid
  /// \return false Packet not valid
  bool packetValid() {
    auto const queue_almost_full{size(queue_) >= queue_.max_size() - 1uz};
    auto const command{packet2command(*cend(queue_))};
    return !busy(command, queue_almost_full) && crcCheck(command);
  }

  /// Check if busy, setup nack/ack transmission
  ///
  /// \param  cmd               Command
  /// \param  queue_almost_full Only 1 slot left in queue
  /// \return true              Busy
  /// \return false             Not busy
  bool busy(Command cmd, bool queue_almost_full) {
    if (cmd == Command::Busy) {
      nack(crc8_);
      if (!crc8_) ack(queue_almost_full);
    }
    return queue_almost_full;
  }

  /// Check if CRC is valid, setup nack/ack transmission
  ///
  /// \param  cmd   Command
  /// \return true  CRC is valid
  /// \return false CRC is not valid
  bool crcCheck(Command cmd) {
    uint32_t crc;
    // Commands with CRC32 also transmit failures in channel2
    if (cmd == Command::FirmwareUpdate || cmd == Command::ZppUpdate) {
      crc = crc32_;
      ack(crc);
    } else {
      crc = crc8_;
      // And there's also an exception for FirmwareSalsa20IV...
      if (cmd == Command::FirmwareSalsa20IV) ack(crc);
    }
    nack(crc);
    return !crc;
  }

  /// Set active status
  ///
  /// \param  active  Active
  void active(bool active) { active_ = active; }

  /// Get selected status
  ///
  /// \return true  Decoder selected
  /// \return false Decoder not selected
  bool selected() const { return selected_; }

  /// Set selected status
  ///
  /// \param  selected  Selected
  void select(bool selected) { selected_ = selected; }

  /// Get nack status
  ///
  /// \return true  Transmit ackbit in channel1
  /// \return false Do not transmit ackbit in channel1
  bool nack() const { return nack_; }

  /// Set nack status
  ///
  /// \param  nack  Nack
  void nack(bool nack) { nack_ = nack; }

  /// Get ack status
  ///
  /// \return true  Transmit ackbit in channel2
  /// \return false Do not transmit ackbit in channel2
  bool ack() const { return ack_; };

  /// Set ack status
  ///
  /// \param  ack Ack
  void ack(bool ack) { ack_ = ack; }

  /// Execute ping (short and long version)
  ///
  /// \param  packet  Packet
  void executePing(Packet const& packet) {
    uint32_t serial_number{};
    uint32_t decoder_id{};
    if (packet.size < 9uz)
      decoder_id = packet.data[4uz]
                     ? static_cast<uint32_t>(packet.data[4uz] << 24u) |
                         (cfg_.decoder_id & 0x00FF'FFFFu)
                     : 0u;
    else {
      serial_number = data2uint32(&packet.data[4uz]);
      if (packet.size >= 12uz) decoder_id = data2uint32(&packet.data[8uz]);
    }
    executePing(serial_number, decoder_id);
  }

  /// Execute ping
  ///
  /// \param  serial_number Serial number
  /// \param  decoder_id    Decoder ID
  void executePing(uint32_t serial_number, uint32_t decoder_id) {
    // Set ack on exit
    gsl::final_action set_ack{[this] { ack(selected_); }};
    if (serial_number && decoder_id)
      select(serial_number == cfg_.serial_number &&
             decoder_id == cfg_.decoder_id);
    else if (serial_number) select(serial_number == cfg_.serial_number);
    else if (decoder_id) select(decoder_id == cfg_.decoder_id);
    else select(true);
  }

  /// Execute config transfer rate
  ///
  /// \param  transfer_rate Transfer rate
  void executeConfigTransferRate(TransferRate transfer_rate) {
    if (transfer_rate < cfg_.transfer_rate) ack(true);
    else if (auto const i{std::to_underlying(transfer_rate)}; i < size(timings))
      transfer_rate_index_ = i;
  }

  /// Execute binary search
  ///
  /// \param  position  Bit position to test
  void executeBinarySearch(uint32_t position) {
    bool const bit{
      binary_search_(cfg_.serial_number, cfg_.decoder_id, position)};
    ack(bit);
  }

  /// Execute CV read
  ///
  /// \param  addr      CV address
  /// \param  position  Bit position to test
  void executeCvRead(uint32_t addr, uint32_t position) {
    bool const bit{readCv(addr, position)};
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

  using Fp = auto (Base::*)(uint32_t, Bit) -> void;
  Fp fp_{&Base::preamble};
  size_t bit_count_{};        ///< Count received bits
  size_t ackreqbit_count_{};  ///< Count received ackreqbits
  Config const cfg_{};
  Crc32 crc32_{};
  ztl::circular_array<Packet, 2uz> queue_{};
  Crc8 crc8_;
  uint8_t transfer_rate_index_{std::to_underlying(TransferRate::Default)};
  uint8_t byte_{};
  BinarySearch binary_search_{};
  bool selected_ : 1 {true};
  bool active_ : 1 {};
  bool nack_ : 1 {};
  bool ack_ : 1 {};
};

}  // namespace mdu::rx::detail