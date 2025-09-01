// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Command station
///
/// \file   mdu/tx/command_station.hpp
/// \author Jonas Gahlert
/// \date   10/03/2025

#pragma once

#include <concepts>

namespace mdu::tx {

template<typename T>
concept CommandStation = requires(T t, size_t ackreq_count) {
  { t.ackreqBegin() } -> std::same_as<void>;
  { t.ackreqChannel1(ackreq_count) } -> std::same_as<void>;
  { t.ackreqChannel2(ackreq_count) } -> std::same_as<void>;
  { t.ackreqEnd() } -> std::same_as<void>;
};

} // namespace mdu::tx
