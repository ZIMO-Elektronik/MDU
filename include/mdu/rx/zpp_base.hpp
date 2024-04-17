// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive ZPP base
///
/// \file   mdu/rx/zpp_base.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include "base.hpp"
#include "mixin/zpp.hpp"

namespace mdu::rx {

using ZppBase = Base<mixin::Zpp>;

}  // namespace mdu::rx