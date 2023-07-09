// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive firmware base
///
/// \file   mdu/rx/firmware_base.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include "../command.hpp"
#include "../crc32.hpp"
#include "../packet.hpp"
#include "base.hpp"

extern "C" {
#include <salsa20/ecrypt-sync.h>
}

namespace mdu::rx {

namespace detail {

ECRYPT_ctx make_salsa20_context(uint32_t decoder_id,
                                std::span<uint8_t const, 8uz> iv,
                                char const* master_key);

/// Provides firmware update logic
struct FirmwareMixin {
  /// Ctor
  ///
  /// \param  salsa20_master_key  Salsa20 master key
  explicit constexpr FirmwareMixin(char const* salsa20_master_key)
    : salsa20_master_key_{salsa20_master_key} {}

  /// Dtor
  virtual constexpr ~FirmwareMixin() = default;

  /// Execute firmware commands
  ///
  /// \param  cmd         Command
  /// \param  packet      Packet
  /// \param  decoder_id  Decoder ID
  /// \return true        Transmit ackbit in channel2
  /// \return false       Do not transmit ackbit in channel2
  bool execute(Command cmd, Packet const& packet, uint32_t decoder_id) {
    switch (cmd) {
      case Command::FirmwareSalsa20IV: {
        std::span<uint8_t const, 8uz> iv{&packet.data[4uz], 8uz};
        return executeSalsa20IV(decoder_id, iv);
      }
      case Command::FirmwareErase: {
        auto const begin_addr{data2uint32(&packet.data[4uz])};
        auto const end_addr{data2uint32(&packet.data[8uz])};
        return executeErase(begin_addr, end_addr);
      }
      case Command::FirmwareUpdate: {
        auto const address{data2uint32(&packet.data[4uz])};
        auto const size{packet.size - sizeof(Command) - sizeof(address) -
                        sizeof(Crc32)};
        assert(size == 64uz);
        std::span<uint8_t const, 64uz> chunk{&packet.data[8uz], 64uz};
        return executeUpdate(address, chunk);
      }
      case Command::FirmwareCrc32Start: {
        auto const begin_addr{data2uint32(&packet.data[4uz])};
        auto const end_addr{data2uint32(&packet.data[8uz])};
        auto const crc32{data2uint32(&packet.data[12uz])};
        return executeCrc32Start(begin_addr, end_addr, crc32);
      }
      case Command::FirmwareCrc32Result: return executeCrc32Result(false);
      case Command::FirmwareCrc32ResultExit: return executeCrc32Result(true);
      default: return false;
    }
  }

private:
  /// Erase firmware in the closed-interval [begin_addr, end_addr[
  ///
  /// \param  begin_addr  Begin address
  /// \param  end_addr    End address
  /// \return true        Success
  /// \return false       Failure
  virtual bool eraseFirmware(uint32_t begin_addr, uint32_t end_addr) = 0;

  /// Write firmware
  ///
  /// \param  addr  Address
  /// \param  chunk Chunk
  /// \return true  Success
  /// \return false Failure
  virtual bool writeFirmware(uint32_t addr,
                             std::span<uint8_t const, 64uz> chunk) = 0;

  /// Exit firmware
  [[noreturn]] virtual void exitFirmware() = 0;

  /// Execute FirmwareSalsa20IV command
  ///
  /// \param  decoder_id  Decoder ID
  /// \param  iv          Initialization vector
  /// \return true        Transmit ackbit in channel2
  /// \return false       Do not transmit ackbit in channel2
  bool executeSalsa20IV(uint32_t decoder_id, std::span<uint8_t const, 8uz> iv) {
    ctx_ = make_salsa20_context(decoder_id, iv, salsa20_master_key_);
    return false;
  }

  /// Execute FirmwareErase command
  ///
  /// \param  begin_addr  Begin address
  /// \param  end_addr    End address
  /// \return true        Transmit ackbit in channel2
  /// \return false       Do not transmit ackbit in channel2
  bool executeErase(uint32_t begin_addr, uint32_t end_addr) {
    auto const success{eraseFirmware(begin_addr, end_addr)};
    return !success;
  }

  /// Execute FirmwareUpdate command
  ///
  /// \param  addr  Address
  /// \param  chunk Chunk
  /// \return true  Transmit ackbit in channel2
  /// \return false Do not transmit ackbit in channel2
  bool executeUpdate(uint32_t addr, std::span<uint8_t const, 64uz> chunk) {
    if (!first_addr_) first_addr_ = addr;
    // Lost packet
    if (last_addr_ && last_addr_ < addr) return true;
    // Already written
    if (last_addr_ && last_addr_ > addr) return false;
    std::array<uint8_t, std::size(chunk)> decrypted_chunk;
    ECRYPT_decrypt_bytes(
      &ctx_, std::data(chunk), data(decrypted_chunk), size(decrypted_chunk));
    if (writeFirmware(addr, decrypted_chunk)) {
      last_addr_ = addr + size(decrypted_chunk);
      crc32_.next(chunk);
      return false;
    }
    return true;
  }

  /// Execute FirmwareCrc32Start command
  ///
  /// \param  begin_addr  Begin address
  /// \param  end_addr    End address
  /// \param  crc32       CRC32
  /// \return true        Transmit ackbit in channel2
  /// \return false       Do not transmit ackbit in channel2
  bool
  executeCrc32Start(uint32_t begin_addr, uint32_t end_addr, uint32_t crc32) {
    if (begin_addr != first_addr_ || end_addr + 1u != last_addr_) return true;
    crc32_valid_ = crc32 == crc32_;
    return false;
  }

  /// Execute FirmwareCrc32Result or FirmwareCrc32ResultExit command
  ///
  /// \param  exit  true  Exit
  ///               false Do not exit
  /// \return true  Transmit ackbit in channel2
  /// \return false Do not transmit ackbit in channel2
  bool executeCrc32Result(bool exit) {
    if (exit && crc32_valid_) {
      exitFirmware();
      return false;
    } else return !crc32_valid_;
  }

  char const* const salsa20_master_key_;
  Crc32 crc32_{};
  ECRYPT_ctx ctx_{};
  std::optional<uint32_t> first_addr_{};
  std::optional<uint32_t> last_addr_{};
  bool crc32_valid_{};
};

}  // namespace detail

using FirmwareBase = detail::Base<detail::FirmwareMixin>;

}  // namespace mdu::rx