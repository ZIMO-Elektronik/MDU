// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive ZPP mixin
///
/// \file   rx/mixin/zpp.cpp
/// \author Vincent Hamp
/// \date   29/10/2023

#include "rx/mixin/zpp.hpp"

namespace mdu::rx::mixin {

/// Execute ZPP commands
///
/// \param  cmd     Command
/// \param  packet  Packet
/// \retval true    Transmit ackbit in channel2
/// \retval false   Do not transmit ackbit in channel2
bool Zpp::execute(Command cmd, Packet const& packet, uint32_t) {
  // The following commands may run without ZPP validation
  switch (cmd) {
    case Command::ZppValidQuery: {
      std::string_view zpp_id{std::bit_cast<char*>(&packet[4uz]), 2uz};
      auto const zpp_flash_size{data2uint32(&packet[6uz])};
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
      std::span<uint8_t const, 4uz> developer_code{&packet[4uz], 4uz};
      return executeLcDcQuery(developer_code);
    }
    case Command::ZppErase: {
      auto const begin_addr{data2uint32(&packet[4uz])};
      auto const end_addr{data2uint32(&packet[8uz])};
      return executeErase(begin_addr, end_addr);
    }
    case Command::ZppUpdate: {
      auto const address{data2uint32(&packet[4uz])};
      auto const bytes_size{size(packet) - sizeof(Command) - sizeof(address) -
                            sizeof(Crc32)};
      return executeUpdate(address, {&packet[8uz], bytes_size});
    }
    case Command::ZppUpdateEnd: {
      auto const begin_addr{data2uint32(&packet[4uz])};
      auto const end_addr{data2uint32(&packet[8uz])};
      return executeEnd(begin_addr, end_addr);
    }
    default: return false;
  }
}

/// Execute ZppValidQuery command
///
/// \param  zpp_id          ZPP ID
/// \param  zpp_flash_size  ZPP flash size
/// \retval true            Transmit ackbit in channel2
/// \retval false           Do not transmit ackbit in channel2
bool Zpp::executeValidQuery(std::string_view zpp_id, size_t zpp_flash_size) {
  _zpp_valid = zppValid(zpp_id, zpp_flash_size);
  return !_zpp_valid;
}

/// Execute ZppLcDcQuery command
///
/// \param  developer_code  Developer code
/// \retval true            Transmit ackbit in channel2
/// \retval false           Do not transmit ackbit in channel2
bool Zpp::executeLcDcQuery(std::span<uint8_t const, 4uz> developer_code) const {
  bool const valid{loadCodeValid(developer_code)};
  return !valid;
}

/// Execute ZppErase command
///
/// \param  begin_addr  Begin address
/// \param  end_addr    End address
/// \retval true        Transmit ackbit in channel2
/// \retval false       Do not transmit ackbit in channel2
bool Zpp::executeErase(uint32_t begin_addr, uint32_t end_addr) {
  auto const success{eraseZpp(begin_addr, end_addr)};
  return !success;
}

/// Execute ZppUpdate command
///
/// \param  addr  Address
/// \param  bytes Bytes
/// \retval true  Transmit ackbit in channel2
/// \retval false Do not transmit ackbit in channel2
bool Zpp::executeUpdate(uint32_t addr, std::span<uint8_t const> bytes) {
  if (!_first_addr) _first_addr = addr;
  // Lost packet
  if (_last_addr && _last_addr < addr) return true;
  // Already written
  if (_last_addr && _last_addr > addr) return false;
  if (writeZpp(addr, bytes)) {
    _last_addr = addr + std::size(bytes);
    return false;
  }
  return true;
}

/// Execute ZppUpdateEnd command
///
/// \param  begin_addr  Begin address
/// \param  end_addr    End address
/// \retval true        Transmit ackbit in channel2
/// \retval false       Do not transmit ackbit in channel2
bool Zpp::executeEnd(uint32_t begin_addr, uint32_t end_addr) {
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
/// \retval true      Transmit ackbit in channel2
/// \retval false     Do not transmit ackbit in channel2
bool Zpp::executeExit(bool reset_cvs) {
  if (_addrs_valid || (!_first_addr && !_last_addr)) {
    exitZpp(reset_cvs);
    return false;
  }
  while (!eraseZpp(*_first_addr, *_last_addr));
  _first_addr = _last_addr = {};
  return true;
}

} // namespace mdu::rx::mixin