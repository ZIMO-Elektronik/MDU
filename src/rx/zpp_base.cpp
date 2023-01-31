// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive ZPP base
///
/// \file   mdu/rx/zpp_base.cpp
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
      auto const zpp_size{data2uint32(&packet.data[6uz])};
      return executeValidQuery(zpp_id, zpp_size);
    }
    case Command::ZppExit: return executeExit(false);
    case Command::ZppExitReset: return executeExit(true);
  }

  // All others may not
  if (!zpp_valid_) return true;
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
      auto const bytes_count{packet.bytes_count - sizeof(Command) -
                             sizeof(address) - sizeof(Crc32)};
      return executeUpdate(address, {&packet.data[8uz], bytes_count});
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
/// \param  zpp_id    ZPP ID
/// \param  zpp_size  ZPP size
/// \return true      Transmit ackbit in channel2
/// \return false     Do not transmit ackbit in channel2
bool ZppMixin::executeValidQuery(std::string_view zpp_id, size_t zpp_size) {
  zpp_valid_ = zppValid(zpp_id, zpp_size);
  return !zpp_valid_;
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
bool ZppMixin::executeErase(uint32_t begin_addr, uint32_t end_addr) const {
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
  if (!first_addr_) first_addr_ = addr;
  // Lost packet
  if (last_addr_ && last_addr_ < addr) return true;
  // Already written
  if (last_addr_ && last_addr_ > addr) return false;
  if (writeZpp(addr, chunk)) {
    last_addr_ = addr + std::size(chunk);
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
  if (!first_addr_ || !last_addr_) return false;
  addrs_valid_ = begin_addr == first_addr_ && end_addr == last_addr_;
  if (!addrs_valid_) return true;
  first_addr_ = last_addr_ = {};
  auto const success{endZpp()};
  return !success;
}

/// Execute ZppExit or ZppExitReset command
///
/// \param  reset_cvs Reset CVs
/// \return true      Transmit ackbit in channel2
/// \return false     Do not transmit ackbit in channel2
bool ZppMixin::executeExit(bool reset_cvs) {
  if (addrs_valid_ || (!first_addr_ && !last_addr_)) {
    exitZpp(reset_cvs);
    return false;
  }
  while (!eraseZpp(*first_addr_, *last_addr_))
    ;
  first_addr_ = last_addr_ = {};
  return true;
}

}  // namespace mdu::rx::detail