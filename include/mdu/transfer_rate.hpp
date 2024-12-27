// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transfer rate
///
/// \file   mdu/transfer_rate.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <cstdint>
#include <utility>

namespace mdu {

/// Supported transfer rates
enum class TransferRate : uint8_t {
  Fallback = 0u,
  Fast = 1u,
  Medium = 2u,
  Slow = 3u,
  Default = 4u
};

constexpr bool operator<(TransferRate lhs, TransferRate rhs) {
  return lhs == TransferRate::Fallback
           ? false
           : std::to_underlying(lhs) < std::to_underlying(rhs);
}

} // namespace mdu