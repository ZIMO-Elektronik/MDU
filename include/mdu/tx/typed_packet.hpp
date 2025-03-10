#pragma once

#include "../packet.hpp"
#include "packet_type.hpp"

namespace mdu::tx {
struct TypedPacket {
  Packet packet;
  PacketType type;
};

} // namespace mdu::tx