// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Command
///
/// \file   mdu/command.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <cstdint>

namespace mdu {

/// Command codes
enum class Command : uint32_t {
  Ping = 0xFFFF'FFFFu,
  ConfigTransferRate = 0xFFFF'FFFEu,
  BinarySearch = 0xFFFF'FFFAu,
  CvRead = 0xFFFF'FFF6u,
  CvWrite = 0xFFFF'FFF9u,
  Busy = 0xFFFF'FFF2u,
  FirmwareSalsa20IV = 0xFFFF'FFF7u,
  FirmwareErase = 0xFFFF'FFF5u,
  FirmwareUpdate = 0xFFFF'FFF8u,
  FirmwareCrc32Start = 0xFFFF'FFFBu,
  FirmwareCrc32Result = 0xFFFF'FFFCu,
  FirmwareCrc32ResultExit = 0xFFFF'FFFDu,
  ZppValidQuery = 0xFFFF'FF06u,
  ZppLcDcQuery = 0xFFFF'FF07u,
  ZppErase = 0xFFFF'FF05u,
  ZppUpdate = 0xFFFF'FF08u,
  ZppUpdateEnd = 0xFFFF'FF0Bu,
  ZppExit = 0xFFFF'FF0Cu,
  ZppExitReset = 0xFFFF'FF0Du
};

}  // namespace mdu