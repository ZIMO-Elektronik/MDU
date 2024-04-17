// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive ZPP mixin
///
/// \file   mdu/rx/mixin/zpp.hpp
/// \author Vincent Hamp
/// \date   29/10/2023

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include "../../command.hpp"
#include "../../packet.hpp"

namespace mdu::rx::mixin {

/// Provides ZPP update logic
struct Zpp {
  /// Dtor
  virtual constexpr ~Zpp() = default;

  bool execute(Command cmd, Packet const& packet, uint32_t);

private:
  /// Check if ZPP is valid
  ///
  /// \param  zpp_id          ZPP ID
  /// \param  zpp_flash_size  ZPP flash size
  /// \return true            ZPP is valid
  /// \return false           ZPP is not valid
  virtual bool zppValid(std::string_view zpp_id,
                        size_t zpp_flash_size) const = 0;

  /// Check if load code is valid
  ///
  /// \param  developer_code  Developer code
  /// \return true            Load code is valid
  /// \return false           Load code is not valid
  virtual bool
  loadCodeValid(std::span<uint8_t const, 4uz> developer_code) const = 0;

  /// Erase ZPP in the closed-interval [begin_addr, end_addr[
  ///
  /// \param  begin_addr  Begin address
  /// \param  end_addr    End address
  /// \return true        Success
  /// \return false       Failure
  virtual bool eraseZpp(uint32_t begin_addr, uint32_t end_addr) = 0;

  /// Write ZPP
  ///
  /// \param  addr  Address
  /// \param  bytes Bytes
  /// \return true  Success
  /// \return false Failure
  virtual bool writeZpp(uint32_t addr, std::span<uint8_t const> bytes) = 0;

  /// Update done
  ///
  /// \return true  Success
  /// \return false Failure
  virtual bool endZpp() = 0;

  /// Exit ZPP
  ///
  /// \param  reset_cvs Reset CVs
  [[noreturn]] virtual void exitZpp(bool reset_cvs) = 0;

  bool executeValidQuery(std::string_view zpp_id, size_t zpp_size);
  bool executeLcDcQuery(std::span<uint8_t const, 4uz> developer_code) const;
  bool executeErase(uint32_t begin_addr, uint32_t end_addr);
  bool executeUpdate(uint32_t addr, std::span<uint8_t const> bytes);
  bool executeEnd(uint32_t begin_addr, uint32_t end_addr);
  bool executeExit(bool reset);

  std::optional<uint32_t> _first_addr{};
  std::optional<uint32_t> _last_addr{};
  bool _addrs_valid : 1 {};
  bool _zpp_valid : 1 {};
};

}  // namespace mdu::rx::mixin
