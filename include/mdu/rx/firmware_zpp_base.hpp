// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive firmware & ZPP base
///
/// \file   mdu/rx/firmware_zpp_base.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include "firmware_base.hpp"
#include "zpp_base.hpp"

namespace mdu::rx {

using FirmwareZppBase = detail::Base<detail::FirmwareMixin, detail::ZppMixin>;

}  // namespace mdu::rx