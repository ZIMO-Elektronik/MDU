// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive ZSU base
///
/// \file   mdu/rx/zsu_base.hpp
/// \author Vincent Hamp
/// \date   29/10/2023

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

/// Provides ZSU update logic
struct ZsuMixin {
  /// Ctor
  ///
  /// \param  salsa20_master_key  Salsa20 master key
  explicit constexpr ZsuMixin(char const* salsa20_master_key)
    : _salsa20_master_key{salsa20_master_key} {}

  /// Dtor
  virtual constexpr ~ZsuMixin() = default;

  /// Execute ZSU commands
  ///
  /// \param  cmd         Command
  /// \param  packet      Packet
  /// \param  decoder_id  Decoder ID
  /// \return true        Transmit ackbit in channel2
  /// \return false       Do not transmit ackbit in channel2
  bool execute(Command cmd, Packet const& packet, uint32_t decoder_id) {
    switch (cmd) {
      case Command::ZsuSalsa20IV: {
        std::span<uint8_t const, 8uz> iv{&packet[4uz], 8uz};
        return executeSalsa20IV(decoder_id, iv);
      }
      case Command::ZsuErase: {
        auto const begin_addr{data2uint32(&packet[4uz])};
        auto const end_addr{data2uint32(&packet[8uz])};
        return executeErase(begin_addr, end_addr);
      }
      case Command::ZsuUpdate: {
        auto const address{data2uint32(&packet[4uz])};
        auto const chunk_size{size(packet) - sizeof(Command) - sizeof(address) -
                              sizeof(Crc32)};
        assert(chunk_size == 64uz);
        std::span<uint8_t const, 64uz> chunk{&packet[8uz], 64uz};
        return executeUpdate(address, chunk);
      }
      case Command::ZsuCrc32Start: {
        auto const begin_addr{data2uint32(&packet[4uz])};
        auto const end_addr{data2uint32(&packet[8uz])};
        auto const crc32{data2uint32(&packet[12uz])};
        return executeCrc32Start(begin_addr, end_addr, crc32);
      }
      case Command::ZsuCrc32Result: return executeCrc32Result(false);
      case Command::ZsuCrc32ResultExit: return executeCrc32Result(true);
      default: return false;
    }
  }

private:
  /// Erase ZSU in the closed-interval [begin_addr, end_addr[
  ///
  /// \param  begin_addr  Begin address
  /// \param  end_addr    End address
  /// \return true        Success
  /// \return false       Failure
  virtual bool eraseZsu(uint32_t begin_addr, uint32_t end_addr) = 0;

  /// Write ZSU
  ///
  /// \param  addr  Address
  /// \param  chunk Chunk
  /// \return true  Success
  /// \return false Failure
  virtual bool writeZsu(uint32_t addr,
                        std::span<uint8_t const, 64uz> chunk) = 0;

  /// Exit ZSU
  [[noreturn]] virtual void exitZsu() = 0;

  /// Execute ZsuSalsa20IV command
  ///
  /// \param  decoder_id  Decoder ID
  /// \param  iv          Initialization vector
  /// \return true        Transmit ackbit in channel2
  /// \return false       Do not transmit ackbit in channel2
  bool executeSalsa20IV(uint32_t decoder_id, std::span<uint8_t const, 8uz> iv) {
    _ctx = make_salsa20_context(decoder_id, iv, _salsa20_master_key);
    return false;
  }

  /// Execute ZsuErase command
  ///
  /// \param  begin_addr  Begin address
  /// \param  end_addr    End address
  /// \return true        Transmit ackbit in channel2
  /// \return false       Do not transmit ackbit in channel2
  bool executeErase(uint32_t begin_addr, uint32_t end_addr) {
    auto const success{eraseZsu(begin_addr, end_addr)};
    return !success;
  }

  /// Execute ZsuUpdate command
  ///
  /// \param  addr  Address
  /// \param  chunk Chunk
  /// \return true  Transmit ackbit in channel2
  /// \return false Do not transmit ackbit in channel2
  bool executeUpdate(uint32_t addr, std::span<uint8_t const, 64uz> chunk) {
    if (!_first_addr) _first_addr = addr;
    // Lost packet
    if (_last_addr && _last_addr < addr) return true;
    // Already written
    if (_last_addr && _last_addr > addr) return false;
    std::array<uint8_t, std::size(chunk)> decrypted_chunk;
    ECRYPT_decrypt_bytes(
      &_ctx, std::data(chunk), data(decrypted_chunk), size(decrypted_chunk));
    if (writeZsu(addr, decrypted_chunk)) {
      _last_addr = addr + size(decrypted_chunk);
      _crc32.next(chunk);
      return false;
    }
    return true;
  }

  /// Execute ZsuCrc32Start command
  ///
  /// \param  begin_addr  Begin address
  /// \param  end_addr    End address
  /// \param  crc32       CRC32
  /// \return true        Transmit ackbit in channel2
  /// \return false       Do not transmit ackbit in channel2
  bool
  executeCrc32Start(uint32_t begin_addr, uint32_t end_addr, uint32_t crc32) {
    if (begin_addr != _first_addr || end_addr + 1u != _last_addr) return true;
    _crc32valid = crc32 == _crc32;
    return false;
  }

  /// Execute ZsuCrc32Result or ZsuCrc32ResultExit command
  ///
  /// \param  exit  true  Exit
  ///               false Do not exit
  /// \return true  Transmit ackbit in channel2
  /// \return false Do not transmit ackbit in channel2
  bool executeCrc32Result(bool exit) {
    if (exit && _crc32valid) {
      exitZsu();
      return false;
    } else return !_crc32valid;
  }

  char const* _salsa20_master_key;
  Crc32 _crc32{};
  ECRYPT_ctx _ctx{};
  std::optional<uint32_t> _first_addr{};
  std::optional<uint32_t> _last_addr{};
  bool _crc32valid{};
};

}  // namespace detail

using ZsuBase = detail::Base<detail::ZsuMixin>;

}  // namespace mdu::rx