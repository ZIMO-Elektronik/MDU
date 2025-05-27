// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Bit
///
/// \file   mdu/bit.hpp
/// \author Vincent Hamp
/// \date   30/01/2023

#pragma once

#include "timing.hpp"

namespace mdu {

enum Bit : uint8_t { _0, _1, Ackreq, Invalid };

/// Convert time to bit
///
/// \param  time                Time is µs
/// \param  transfer_rate_index Current index of transfer rate
/// \retval _0                  Time is a zero bit
/// \retval _1                  Time is a one bit
/// \retval Ackreq              Time is a ackreq bit
/// \retval Invalid             Time isn't valid
constexpr Bit time2bit(uint32_t time, size_t transfer_rate_index) {
  if (is_zero(time, transfer_rate_index)) return _0;
  else if (is_one(time, transfer_rate_index)) return _1;
  else if (is_ackreq(time, transfer_rate_index)) return Ackreq;
  else return Invalid;
}

} // namespace mdu
