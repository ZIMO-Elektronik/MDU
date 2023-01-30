// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Utility
///
/// \file   mdu/utility.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <cstdint>

namespace mdu {

/// Data to uint16_t
///
/// \param  data  Pointer to data
/// \return uint16_t from data
constexpr auto data2uint16(uint8_t const* data) {
  return static_cast<uint16_t>(data[0uz] << 8u | data[1uz] << 0u);
}

/// Data to uint32_t
///
/// \param  data  Pointer to data
/// \return uint32_t from data
constexpr auto data2uint32(uint8_t const* data) {
  return static_cast<uint32_t>(data[0uz] << 24u | data[1uz] << 16u |
                               data[2uz] << 8u | data[3uz] << 0u);
}

/// Data to uint64_t
///
/// \param  data  Pointer to data
/// \return uint64_t from data
constexpr auto data2uint64(uint8_t const* data) {
  auto const upper{data2uint32(data)};
  auto const lower{data2uint32(data + 4)};
  return static_cast<uint64_t>(upper) << 32u | lower;
}

}  // namespace mdu