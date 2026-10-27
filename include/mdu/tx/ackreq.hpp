// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Ackreq
///
/// \file   mdu/tx/ackreq.hpp
/// \author Vincent Hamp
/// \date   27/10/2026

#pragma once

#include <concepts>
#include <cstddef>

namespace mdu::tx {

template<typename T>
concept Ackreq = requires(T t, size_t ackreq_count) {
  { t.ackreqBegin() } -> std::same_as<void>;
  { t.ackreqChannel1(ackreq_count) } -> std::same_as<void>;
  { t.ackreqChannel2(ackreq_count) } -> std::same_as<void>;
  { t.ackreqEnd() } -> std::same_as<void>;
};

} // namespace mdu::tx
