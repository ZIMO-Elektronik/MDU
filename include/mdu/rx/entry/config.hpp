// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive entry config
///
/// \file   mdu/rx/entry/config.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <cstdint>
#include <functional>

namespace mdu::rx::entry {

struct Config {
  uint32_t serial_number{};
  uint32_t decoder_id{};
  std::function<void()> zpp_entry{};
  std::function<void()> zsu_entry{};
};

} // namespace mdu::rx::entry
