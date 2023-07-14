// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Packet
///
/// \file   mdu/packet.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <ztl/inplace_vector.hpp>
#include "command.hpp"
#include "crc32.hpp"
#include "utility.hpp"

namespace mdu {

using Packet = ztl::inplace_vector<uint8_t,
                                   sizeof(Command) + sizeof(uint32_t) + 256uz +
                                     sizeof(Crc32)>;

/// Packet to command
///
/// \param  packet  Packet
/// \return Command
constexpr Command packet2command(Packet const& packet) {
  return static_cast<Command>(data2uint32(data(packet)));
}

}  // namespace mdu