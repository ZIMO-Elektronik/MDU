// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transfer phase
///
/// \file   mdu/tx/phase.hpp
/// \author Jonas Gahlert
/// \date   10.03.2025

#pragma once

namespace mdu::tx::detail {

/// Transfer phases
enum class Phase { Idle, Preamble, Packet, PacketEnd, ACKreq };

} // namespace mdu::tx::detail