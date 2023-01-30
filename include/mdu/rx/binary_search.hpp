// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Binary search
///
/// \file   mdu/rx/binary_search.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <cstdint>

namespace mdu::rx {

/// Binary search
struct BinarySearch {
  bool
  operator()(uint32_t serial_number, uint32_t decoder_id, uint32_t bit_number);

private:
  bool ignore_{true};
};

}  // namespace mdu::rx