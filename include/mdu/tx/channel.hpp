// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit base
///
/// \file   mdu/tx/channel.hpp
/// \author Jonas Gahlert
/// \date   10.03.2025

#pragma once

namespace mdu::tx::detail {

/// Is reference
///
/// \param [in] bit ACKreq bit pos
/// \retval         true is Reference bit
/// \retval         false is not Reference bit
inline constexpr bool is_reference(int bit) { return (bit == 0) || (bit == 1); }

/// Is channel 1
///
/// \param [in] bit ACKreq bit pos
/// \retval         true is channel 1 bit
/// \retval         false is not channel 1 bit
inline constexpr bool is_channel1(int bit) { return (bit >= 2) && (bit <= 4); }

/// Is channel 2
///
/// @param [in] bit ACKreq bit pos
/// \retval         true is channel 2 bit
/// \retval         false is not channel 2 bit
inline constexpr bool is_channel2(int bit) { return (bit >= 6) && (bit <= 8); }

} // namespace mdu::tx::detail