// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit base
///
/// \file   mdu/tx/command_station.hpp
/// \author Jonas Gahlert
/// \date   10.03.2025

#pragma once

#include <concepts>

namespace mdu::tx {

template<typename T>
concept CommandStation = requires(T t, int I) {
  { t.ackreqBegin() } -> std::same_as<void>;
  { t.ackreqBit(I) } -> std::same_as<bool>;
  { t.ackreqEnd() } -> std::same_as<void>;
};

} // namespace mdu::tx