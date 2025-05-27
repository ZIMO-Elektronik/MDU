// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Binary tree search
///
/// \file   rx/binary_tree_search.cpp
/// \author Vincent Hamp
/// \date   12/12/2022

#include "rx/binary_tree_search.hpp"

namespace mdu::rx {

/// Binary tree search
///
/// \param  serial_number Serial number
/// \param  decoder_id    Decoder ID
/// \param  pos           Bit position
/// \retval true          Transmit ackbit in channel2
/// \retval false         Do not transmit ackbit in channel2
bool BinaryTreeSearch::operator()(uint32_t serial_number,
                                  uint32_t decoder_id,
                                  uint32_t pos) {
  if (pos == 255u) return !(_ignore = false);
  if (_ignore) return false;

  // Set
  if (auto const number{static_cast<uint64_t>(decoder_id) << 32u |
                        serial_number};
      pos <= 62u)
    return number & (static_cast<uint64_t>(1u) << pos);
  // Clear
  else if (pos >= 64u && pos <= 64u + 62u)
    return !(number & (static_cast<uint64_t>(1u) << (pos - 64u)));
  // Ignore from now on if set
  else if (pos >= 128u && pos <= 128u + 62u)
    _ignore = number & (static_cast<uint64_t>(1u) << (pos - 128u));
  // Ignore from now on if clear
  else if (pos >= 192u && pos <= 192u + 62u)
    _ignore = !(number & (static_cast<uint64_t>(1u) << (pos - 192u)));

  return false;
}

} // namespace mdu::rx
