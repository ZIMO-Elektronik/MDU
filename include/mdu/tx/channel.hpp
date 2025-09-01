// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// ACKReq channel helper
///
/// \file   mdu/tx/channel.hpp
/// \author Jonas Gahlert
/// \date   10/03/2025

#pragma once

#include <cstddef>

namespace mdu::tx::detail {

/// Is reference
///
/// \param  bit   ACKreq bit pos
/// \retval true  Is reference bit
/// \retval false Is not reference bit
constexpr bool is_reference(size_t bit) { return bit == 0uz || bit == 1uz; }

/// Is channel 1
///
/// \param  bit   ACKreq bit pos
/// \retval true  Is channel 1 bit
/// \retval false Is not channel 1 bit
constexpr bool is_channel1(size_t bit) { return bit >= 2uz && bit <= 4uz; }

/// Is channel 2
///
/// \param  bit   ACKreq bit pos
/// \retval true  is channel 2 bit
/// \retval false is not channel 2 bit
constexpr bool is_channel2(size_t bit) { return bit >= 6uz && bit <= 8uz; }

} // namespace mdu::tx::detail
