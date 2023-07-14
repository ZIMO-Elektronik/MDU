// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive ZPP base
///
/// \file   rx/zpp_base.cpp
/// \author Vincent Hamp
/// \date   12/12/2022

#include "rx/zpp_base.hpp"

namespace mdu::rx::detail {

/// Execute ZPP commands
///
/// \param  cmd     Command
/// \param  packet  Packet
/// \return true    Transmit ackbit in channel2
/// \return false   Do not transmit ackbit in channel2
bool ZppMixin::execute(Command cmd, Packet const& packet, uint32_t) {
  // The following commands may run without ZPP validation
  switch (cmd) {
    case Command::ZppValidQuery: {
      std::string_view zpp_id{std::bit_cast<char*>(&packet.data[4uz]), 2uz};
      auto const zpp_flash_size{data2uint32(&packet.data[6uz])};
      return executeValidQuery(zpp_id, zpp_flash_size);
    }
    case Command::ZppExit: return executeExit(false);
    case Command::ZppExitReset: return executeExit(true);
    default: break;
  }

  // All others may not
  if (!_zpp_valid) return true;
  switch (cmd) {
    case Command::ZppLcDcQuery: {
      std::span<uint8_t const, 4uz> developer_code{&packet.data[4uz], 4uz};
      return executeLcDcQuery(developer_code);
    }
    case Command::ZppErase: {
      auto const begin_addr{data2uint32(&packet.data[4uz])};
      auto const end_addr{data2uint32(&packet.data[8uz])};
      return executeErase(begin_addr, end_addr);
    }
    case Command::ZppUpdate: {
      auto const address{data2uint32(&packet.data[4uz])};
      auto const size{packet.size - sizeof(Command) - sizeof(address) -
                      sizeof(Crc32)};
      return executeUpdate(address, {&packet.data[8uz], size});
    }
    case Command::ZppUpdateEnd: {
      auto const begin_addr{data2uint32(&packet.data[4uz])};
      auto const end_addr{data2uint32(&packet.data[8uz])};
      return executeEnd(begin_addr, end_addr);
    }
    default: return false;
  }
}

/// Execute ZppValidQuery command
///
/// \param  zpp_id          ZPP ID
/// \param  zpp_flash_size  ZPP flash size
/// \return true            Transmit ackbit in channel2
/// \return false           Do not transmit ackbit in channel2
bool ZppMixin::executeValidQuery(std::string_view zpp_id,
                                 size_t zpp_flash_size) {
  _zpp_valid = zppValid(zpp_id, zpp_flash_size);
  return !_zpp_valid;
}

/// Execute ZppLcDcQuery command
///
/// \param  developer_code  Developer code
/// \return true            Transmit ackbit in channel2
/// \return false           Do not transmit ackbit in channel2
bool ZppMixin::executeLcDcQuery(
  std::span<uint8_t const, 4uz> developer_code) const {
  bool const valid{loadcodeValid(developer_code)};
  return !valid;
}

/// Execute ZppErase command
///
/// \param  begin_addr  Begin address
/// \param  end_addr    End address
/// \return true        Transmit ackbit in channel2
/// \return false       Do not transmit ackbit in channel2
bool ZppMixin::executeErase(uint32_t begin_addr, uint32_t end_addr) {
  auto const success{eraseZpp(begin_addr, end_addr)};
  return !success;
}

/// Execute ZppUpdate command
///
/// \param  addr  Address
/// \param  chunk Chunk
/// \return true  Transmit ackbit in channel2
/// \return false Do not transmit ackbit in channel2
bool ZppMixin::executeUpdate(uint32_t addr, std::span<uint8_t const> chunk) {
  if (!_first_addr) _first_addr = addr;
  // Lost packet
  if (_last_addr && _last_addr < addr) return true;
  // Already written
  if (_last_addr && _last_addr > addr) return false;
  if (writeZpp(addr, chunk)) {
    _last_addr = addr + std::size(chunk);
    return false;
  }
  return true;
}

/// Execute ZppUpdateEnd command
///
/// \param  begin_addr  Begin address
/// \param  end_addr    End address
/// \return true        Transmit ackbit in channel2
/// \return false       Do not transmit ackbit in channel2
bool ZppMixin::executeEnd(uint32_t begin_addr, uint32_t end_addr) {
  if (!_first_addr || !_last_addr) return false;
  _addrs_valid = begin_addr == _first_addr && end_addr == _last_addr;
  if (!_addrs_valid) return true;
  _first_addr = _last_addr = {};
  auto const success{endZpp()};
  return !success;
}

/// Execute ZppExit or ZppExitReset command
///
/// \param  reset_cvs Reset CVs
/// \return true      Transmit ackbit in channel2
/// \return false     Do not transmit ackbit in channel2
bool ZppMixin::executeExit(bool reset_cvs) {
  if (_addrs_valid || (!_first_addr && !_last_addr)) {
    exitZpp(reset_cvs);
    return false;
  }
  while (!eraseZpp(*_first_addr, *_last_addr))
    ;
  _first_addr = _last_addr = {};
  return true;
}

}  // namespace mdu::rx::detail