// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Binary search
///
/// \file   mdu/rx/binary_search.cpp
/// \author Vincent Hamp
/// \date   12/12/2022

#include "rx/binary_search.hpp"

namespace mdu::rx {

/// Binary search
///
/// \param  serial_number Serial number
/// \param  decoder_id    Decoder ID
/// \param  position      Bit position
/// \return true          Transmit ackbit in channel2
/// \return false         Do not transmit ackbit in channel2
bool BinarySearch::operator()(uint32_t serial_number,
                              uint32_t decoder_id,
                              uint32_t position) {
  if (position == 255u) return !(ignore_ = false);
  if (ignore_) return false;

  // Set
  if (auto const number{static_cast<uint64_t>(decoder_id) << 32u |
                        serial_number};
      position <= 62u)
    return number & (static_cast<uint64_t>(1u) << position);
  // Clear
  else if (position >= 64u && position <= 64u + 62u)
    return !(number & (static_cast<uint64_t>(1u) << (position - 64u)));
  // Ignore from now on if set
  else if (position >= 128u && position <= 128u + 62u)
    ignore_ = number & (static_cast<uint64_t>(1u) << (position - 128u));
  // Ignore from now on if clear
  else if (position >= 192u && position <= 192u + 62u)
    ignore_ = !(number & (static_cast<uint64_t>(1u) << (position - 192u)));

  return false;
}

}  // namespace mdu::rx