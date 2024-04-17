// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive ZSU base
///
/// \file   mdu/rx/zsu_base.hpp
/// \author Vincent Hamp
/// \date   29/10/2023

#pragma once

#include "base.hpp"
#include "mixin/zsu.hpp"

namespace mdu::rx {

using ZsuBase = Base<mixin::Zsu>;

}  // namespace mdu::rx