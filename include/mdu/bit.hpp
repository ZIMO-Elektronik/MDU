// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Bit
///
/// \file   mdu/bit.hpp
/// \author Vincent Hamp
/// \date   30/01/2023

#pragma once

#include <cstdint>

namespace mdu {

enum Bit : uint8_t { _0, _1, Ackreq, Invalid };

}  // namespace mdu