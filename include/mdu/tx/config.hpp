// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit base
///
/// \file   mdu/tx/config.hpp
/// \author Jonas Gahlert
/// \date   10.03.2025

#pragma once

#include "../timing.hpp"

namespace mdu::tx {

struct Config {
  uint8_t num_preamble{MDU_TX_MIN_PREAMBLE_BITS};

  uint8_t num_ackreq{MDU_TX_MIN_ACKREQ_BITS};

  bool invert{};
};

} // namespace mdu::tx