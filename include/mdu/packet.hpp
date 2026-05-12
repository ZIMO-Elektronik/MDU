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

/// Make Ping packet
///
/// \param decoder_id Decoder ID
/// \return Packet
constexpr auto make_ping_packet(uint8_t decoder_id) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(decoder_id) + sizeof(Crc8));
  uint32_2data(std::to_underlying(Command::Ping), begin(packet));
  packet[4uz] = decoder_id;
  packet[5uz] = crc8({cbegin(packet), 5uz});
  return packet;
}

/// Make Ping packet
///
/// \param serial_number  Decoder serial number
/// \param decoder_id     Decoder ID
/// \return Packet
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

/// Make config transfer rate packet
///
/// \param transfer_rate Transfer Rate
/// \return Packet
constexpr auto make_config_transfer_rate_packet(TransferRate transfer_rate) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(transfer_rate) + sizeof(Crc8));
  uint32_2data(std::to_underlying(Command::ConfigTransferRate), begin(packet));
  packet[4uz] = std::to_underlying(transfer_rate);
  packet[5uz] = crc8({cbegin(packet), 5uz});
  return packet;
}

/// Make binary tree search packet
///
/// \param byte Byte
/// \return Packet
constexpr auto make_binary_tree_search_packet(uint8_t byte) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(byte) + sizeof(Crc8));
  uint32_2data(std::to_underlying(Command::BinaryTreeSearch), begin(packet));
  packet[4uz] = byte;
  packet[5uz] = crc8({cbegin(packet), 5uz});
  return packet;
}

/// Make CV read packet
///
/// \param cv_number  CV number
/// \param pos        Bit position
/// \return Packet
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

/// Make CV write packet
///
/// \param cv_number  CV number
/// \param byte       CV value
/// \return Packet
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

/// Make busy packet
///
/// \return Packet
constexpr auto make_busy_packet() {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(Crc8));
  uint32_2data(std::to_underlying(Command::Busy), begin(packet));
  packet[4uz] = crc8({cbegin(packet), 4uz});
  return packet;
}

/// Make ZSU salsa20 IV packet
///
/// \param iv Salsa20 IV
/// \return Packet
constexpr auto make_zsu_salsa20_iv_packet(std::span<uint8_t const, 8uz> iv) {
  Packet packet{};
  packet.resize(sizeof(Command) + size(iv) + sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZsuSalsa20IV), first)};
  last = std::copy(cbegin(iv), cend(iv), last);
  *last = crc8({first, last});
  return packet;
}

/// Make ZSU erase packet
///
/// \param begin_addr Begin address
/// \param end_addr   End address
/// \return Packet
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

/// Make ZSU update packet
///
/// \param addr   Block address
/// \param bytes  Block
/// \return Packet
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

/// Make CRC32 start packet
///
/// \param begin_addr Begin address
/// \param end_addr   End address
/// \param crc        CRC32
/// \return Packet
constexpr auto make_zsu_crc32_start_packet(uint32_t begin_addr,
                                           uint32_t end_addr,
                                           uint32_t crc) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(begin_addr) + sizeof(end_addr) +
                sizeof(crc) + sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZsuCrc32Start), first)};
  last = uint32_2data(begin_addr, last);
  last = uint32_2data(end_addr, last);
  last = uint32_2data(crc, last);
  *last = crc8({first, last});
  return packet;
}

/// Make ZSU CRC32 result packet
///
/// \return Packet
constexpr auto make_zsu_crc32_result_packet() {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZsuCrc32Result), first)};
  *last = crc8({first, last});
  return packet;
}

/// Make ZSU CRC32 result and exit packet
///
/// \return Packet
constexpr auto make_zsu_crc32_result_exit_packet() {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(Crc8));
  auto first{begin(packet)};
  auto last{
    uint32_2data(std::to_underlying(Command::ZsuCrc32ResultExit), first)};
  *last = crc8({first, last});
  return packet;
}

/// Make ZPP valid query
///
/// \param zpp_id         ZPP ID
/// \param zpp_flash_size ZPP flash size
/// \return Packet
constexpr auto make_zpp_valid_query_packet(std::string_view zpp_id,
                                           uint32_t zpp_flash_size) {
  Packet packet{};
  packet.resize(static_cast<Packet::size_type>(sizeof(Command) + size(zpp_id) +
                                               sizeof(zpp_flash_size)));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZppValidQuery), first)};
  last = std::copy(begin(zpp_id), end(zpp_id), last);
  last = uint32_2data(zpp_flash_size, last);
  *last = crc8({first, last});
  return packet;
}

/// Make ZPP LC DC query packet
///
/// \param developer_code Developer code
/// \return Packet
constexpr auto
make_zpp_lc_dc_query_packet(std::span<uint8_t const, 4uz> developer_code) {
  Packet packet{};
  packet.resize(sizeof(Command) + size(developer_code) + sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZppLcDcQuery), first)};
  last = std::copy(begin(developer_code), end(developer_code), last);
  *last = crc8({first, last});
  return packet;
}

/// Make ZPP erase packet
///
/// \param begin_addr Begin address
/// \param end_addr   End address
/// \return Packet
constexpr auto make_zpp_erase_packet(uint32_t begin_addr, uint32_t end_addr) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(begin_addr) + sizeof(end_addr) +
                sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZppErase), first)};
  last = uint32_2data(begin_addr, last);
  last = uint32_2data(end_addr, last);
  *last = crc8({first, last});
  return packet;
}

/// Make ZPP update packet
///
/// \param addr   Block address
/// \param bytes  Block
/// \return Packet
constexpr auto make_zpp_update_packet(uint32_t addr,
                                      std::span<uint8_t const, 256uz> bytes) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(addr) + size(bytes) + sizeof(Crc32));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZppUpdate), first)};
  last = uint32_2data(addr, last);
  last = std::copy(cbegin(bytes), cend(bytes), last);
  last = uint32_2data(crc32({first, last}), last);
  return packet;
}

/// Make ZPP update end packet
///
/// \param begin_addr Begin address
/// \param end_addr   End address
/// \return Packet
constexpr auto make_zpp_update_end_packet(uint32_t begin_addr,
                                          uint32_t end_addr) {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(begin_addr) + sizeof(end_addr) +
                sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZppUpdateEnd), first)};
  last = uint32_2data(begin_addr, last);
  last = uint32_2data(end_addr, last);
  *last = crc8({first, last});
  return packet;
}

/// Make ZPP exit packet
///
/// \return Packet
constexpr auto make_zpp_exit_packet() {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZppExit), first)};
  *last = crc8({first, last});
  return packet;
}

/// Make ZPP exit and reset packet
///
/// \return Packet
constexpr auto make_zpp_exit_reset_packet() {
  Packet packet{};
  packet.resize(sizeof(Command) + sizeof(Crc8));
  auto first{begin(packet)};
  auto last{uint32_2data(std::to_underlying(Command::ZppExitReset), first)};
  *last = crc8({first, last});
  return packet;
}

} // namespace mdu
