// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive mixin concept
///
/// \file   mdu/rx/mixin.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <type_traits>
#include "../command.hpp"
#include "../packet.hpp"

namespace mdu::rx::detail {

template<typename T>
concept Mixin = std::is_invocable_r_v<bool,
                                      decltype(&T::execute),
                                      T*,
                                      Command,
                                      Packet const&,
                                      uint32_t>;

}  // namespace mdu::rx::detail