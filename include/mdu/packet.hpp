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
#include <string_view>
#include <ztl/inplace_vector.hpp>
#include "command.hpp"
#include "crc32.hpp"
#include "transfer_rate.hpp"
#include "utility.hpp"

namespace mdu {

using Packet = ztl::inplace_vector<uint8_t, MDU_MAX_PACKET_SIZE>;

/// Packet to command
///
/// \param  packet  Packet
/// \return Command
constexpr Command packet2command(Packet const& packet) {
  return static_cast<Command>(data2uint32(data(packet)));
}

/// TODO
constexpr auto make_ping_packet(uint8_t decoder_id) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(decoder_id) + sizeof(Crc8));
  uint32_2data(std::to_underlying(Command::Ping), begin(packet));
  packet[4uz] = decoder_id;
  packet[5uz] = crc8({cbegin(packet), 5uz});
  return packet;
}

/// TODO
constexpr auto make_ping_packet(uint32_t serial_number, uint32_t decoder_id) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(serial_number) + sizeof(decoder_id) +
                sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::Ping), first)};
  last = uint32_2data(serial_number, last);
  last = uint32_2data(decoder_id, last);
  *last = crc8({first, last});
  return packet;
}

/// TODO
constexpr auto make_config_transfer_rate_packet(TransferRate transfer_rate) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(transfer_rate) + sizeof(Crc8));
  uint32_2data(std::to_underlying(Command::ConfigTransferRate), begin(packet));
  packet[4uz] = std::to_underlying(transfer_rate);
  packet[5uz] = crc8({cbegin(packet), 5uz});
  return packet;
}

/// TODO
constexpr auto make_binary_tree_search_packet(uint8_t byte) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(byte) + sizeof(Crc8));
  uint32_2data(std::to_underlying(Command::BinaryTreeSearch), begin(packet));
  packet[4uz] = byte;
  packet[5uz] = crc8({cbegin(packet), 5uz});
  return packet;
}

/// TODO
constexpr auto make_cv_read_packet(uint16_t cv_number, uint8_t pos) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(cv_number) + sizeof(pos) +
                sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::CvRead), first)};
  last = uint16_2data(cv_number, last);
  *last++ = pos;
  *last = crc8({first, last});
  return packet;
}

/// TODO
constexpr auto make_cv_write_packet(uint16_t cv_number, uint8_t byte) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(cv_number) + sizeof(byte) +
                sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::CvRead), first)};
  last = uint16_2data(cv_number, last);
  *last++ = byte;
  *last = crc8({first, last});
  return packet;
}

/// TODO
constexpr auto make_busy_packet() {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(Crc8));
  uint32_2data(std::to_underlying(Command::Busy), begin(packet));
  packet[4uz] = crc8({cbegin(packet), 4uz});
  return packet;
}

/// TODO
constexpr auto make_zsu_salsa20_iv_packet(std::span<uint8_t const, 8uz> iv) {
  Packet packet{};
  packet.resize(sizeof(Command) + size(iv) + sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZsuSalsa20IV), first)};
  last = std::copy(cbegin(iv), cend(iv), last);
  *last = crc8({first, last});
  return packet;
}

/// TODO
constexpr auto make_zsu_erase_packet(uint32_t begin_addr, uint32_t end_addr) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(begin_addr) + sizeof(end_addr) +
                sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZsuErase), first)};
  last = uint32_2data(begin_addr, last);
  last = uint32_2data(end_addr, last);
  *last = crc8({first, last});
  return packet;
}

/// TODO
constexpr auto make_zsu_update_packet(uint32_t addr,
                                      std::span<uint8_t const, 64uz> bytes) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(addr) + size(bytes) + sizeof(Crc32));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZsuUpdate), first)};
  last = std::copy(cbegin(bytes), cend(bytes), last);
  uint32_2data(crc32({first, last}), last);
  return packet;
}

/// TODO
constexpr auto make_zpp_valid_query_packet(std::string_view zpp_id,
                                           size_t zpp_flash_size) {
  Packet packet{};
  // TODO
  return packet;
}

/// TODO
constexpr auto
make_zpp_lc_dc_query_packet(std::span<uint8_t const, 4uz> developer_code) {
  Packet packet{};
  // TODO
  return packet;
}

/// TODO
constexpr auto make_zpp_update_packet(uint32_t addr,
                                      std::span<uint8_t const> bytes) {
  Packet packet{};
  // TODO
  return packet;
}

/// TODO
constexpr auto make_zpp_update_end_packet(uint32_t begin_addr,
                                          uint32_t end_addr) {
  Packet packet{};
  // TODO
  return packet;
}

/// TODO
constexpr auto make_zpp_exit_packet() {
  Packet packet{};
  // TODO
  return packet;
}

} // namespace mdu
