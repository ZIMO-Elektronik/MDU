#include "packet_builder.hpp"
#include <algorithm>

PacketBuilder& PacketBuilder::preamble(size_t count) {
  _preamble_count = count;
  return *this;
}

PacketBuilder& PacketBuilder::command(mdu::Command cmd) {
  return data(std::to_underlying(cmd));
}

PacketBuilder& PacketBuilder::data(std::span<uint8_t const> bytes) {
  _data.insert(end(_data), cbegin(bytes), cend(bytes));
  return *this;
}

PacketBuilder& PacketBuilder::data(std::string_view sv) {
  _data.insert(end(_data), cbegin(sv), cend(sv));
  return *this;
}

PacketBuilder& PacketBuilder::crc8(std::optional<uint8_t> overwrite_crc8) {
  return data(overwrite_crc8.value_or(mdu::crc8(_data)));
}

PacketBuilder& PacketBuilder::crc32(std::optional<uint32_t> overwrite_crc32) {
  return data(overwrite_crc32.value_or(mdu::crc32(_data)));
}

PacketBuilder& PacketBuilder::ackreq(size_t count) {
  _ackreq_count = count;
  return *this;
}

std::vector<uint32_t>
PacketBuilder::timings(mdu::TransferRate transfer_rate) const {
  std::vector<uint32_t> retval{timingsWithoutAckreq(transfer_rate)};
  auto const& timings{mdu::timings[std::to_underlying(transfer_rate)]};

  // Ackreq
  for (auto i{0uz}; i < _ackreq_count; ++i) retval.push_back(timings.ackreq);

  return retval;
}

std::vector<uint32_t>
PacketBuilder::timingsWithoutAckreq(mdu::TransferRate transfer_rate) const {
  std::vector<uint32_t> retval;
  auto const& timings{mdu::timings[std::to_underlying(transfer_rate)]};

  // Preamble
  for (auto i{0uz}; i < _preamble_count; ++i) retval.push_back(timings.one);

  // Data
  std::ranges::for_each(_data, [&](uint8_t byte) {
    retval.push_back(timings.zero); // Start
    for (auto i{sizeof(byte) * CHAR_BIT}; i-- > 0uz;)
      retval.push_back(byte & (1u << i) ? timings.one : timings.zero);
  });

  // End
  if (size(_data)) retval.push_back(timings.one);

  return retval;
}

std::vector<uint32_t>
PacketBuilder::timingsAckreqOnly(size_t count,
                                 mdu::TransferRate transfer_rate) const {
  return std::vector<uint32_t>(
    count, mdu::timings[std::to_underlying(transfer_rate)].ackreq);
}

PacketBuilder PacketBuilder::makePingPacket(uint8_t decoder_id) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::Ping)
    .data(decoder_id)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder PacketBuilder::makePingPacket(uint32_t serial_number,
                                            uint32_t decoder_id) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::Ping)
    .data(serial_number, decoder_id)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder
PacketBuilder::makeConfigTransferRatePacket(mdu::TransferRate transfer_rate) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ConfigTransferRate)
    .data(std::to_underlying(transfer_rate))
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder PacketBuilder::makeBusyPacket() {
  PacketBuilder packet;
  packet.preamble().command(mdu::Command::Busy).crc8().ackreq();
  return packet;
}

PacketBuilder
PacketBuilder::makeZsuUpdatePacket(uint32_t addr,
                                   std::span<uint8_t const, 64uz> bytes) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZsuUpdate)
    .data(addr)
    .data(bytes)
    .crc32()
    .ackreq();
  return packet;
}

PacketBuilder PacketBuilder::makeZppValidQueryPacket(std::string_view zpp_id,
                                                     size_t zpp_flash_size) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZppValidQuery)
    .data(zpp_id)
    .data(zpp_flash_size)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder PacketBuilder::makeZppLcDcQueryPacket(
  std::span<uint8_t const, 4uz> developer_code) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZppLcDcQuery)
    .data(developer_code)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder
PacketBuilder::makeZppUpdatePacket(uint32_t addr,
                                   std::span<uint8_t const> bytes) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZppUpdate)
    .data(addr)
    .data(bytes)
    .crc32()
    .ackreq();
  return packet;
}

PacketBuilder PacketBuilder::makeZppUpdateEndPacket(uint32_t begin_addr,
                                                    uint32_t end_addr) {
  PacketBuilder packet;
  packet.preamble()
    .command(mdu::Command::ZppUpdateEnd)
    .data(begin_addr, end_addr)
    .crc8()
    .ackreq();
  return packet;
}

PacketBuilder PacketBuilder::makeZppExitPacket() {
  PacketBuilder packet;
  packet.preamble().command(mdu::Command::ZppExit).crc8().ackreq();
  return packet;
}
