// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive config
///
/// \file   mdu/rx/config.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <cstdint>
#include "../transfer_rate.hpp"

namespace mdu::rx {

struct Config {
  uint32_t serial_number{};
  uint32_t decoder_id{};
  TransferRate transfer_rate{TransferRate::Default};
};

} // namespace mdu::rx
