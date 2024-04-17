// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive mixin concept
///
/// \file   mdu/rx/mixin/executable.hpp
/// \author Vincent Hamp
/// \date   17/04/2024

#pragma once

#include <type_traits>
#include "../../command.hpp"
#include "../../packet.hpp"

namespace mdu::rx::mixin {

/// Mixins must be executable
template<typename T>
concept Executable = std::is_invocable_r_v<bool,
                                           decltype(&T::execute),
                                           T*,
                                           Command,
                                           Packet const&,
                                           uint32_t>;

}  // namespace mdu::rx::mixin