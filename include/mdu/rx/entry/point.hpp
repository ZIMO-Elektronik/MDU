// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive entry point
///
/// \file   mdu/rx/entry/point.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <array>
#include <ztl/inplace_deque.hpp>
#include "config.hpp"

namespace mdu::rx::entry {

/// Entry point with CV verify
struct Point {
  Point(Config cfg) : _cfg{cfg} {}
  void verify(size_t index, uint8_t value);

private:
  void
  verifySequence(std::array<std::pair<uint8_t, uint8_t>, 5uz> const& sequence,
                 std::function<void()> const& f);
  Config const _cfg;
  ztl::inplace_deque<std::pair<uint8_t, uint8_t>, 7uz> _deque{};
};

}  // namespace mdu::rx::entry