// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Binary tree search
///
/// \file   mdu/rx/binary_tree_search.hpp
/// \author Vincent Hamp
/// \date   02/08/2023

#pragma once

#include <cstdint>

namespace mdu::rx {

/// Binary tree search
struct BinaryTreeSearch {
  bool
  operator()(uint32_t serial_number, uint32_t decoder_id, uint32_t bit_number);

private:
  bool _ignore{true};
};

} // namespace mdu::rx