// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive packet
///
/// \file   mdu/rx/packet.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "../command.hpp"
#include "../crc32.hpp"
#include "../utility.hpp"

namespace mdu::rx {

/// Receive packet
struct Packet {
  std::array<uint8_t,
             sizeof(Command) + sizeof(uint32_t) + 256uz + sizeof(Crc32)>
    data{};
  size_t bytes_count{};
};

/// Packet to command
///
/// \param  packet  Packet
/// \return Command
constexpr Command packet2command(Packet const& packet) {
  return static_cast<Command>(data2uint32(cbegin(packet.data)));
}

}  // namespace mdu::rx