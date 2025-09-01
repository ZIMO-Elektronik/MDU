#pragma once

#include <concepts>
#include <mdu/mdu.hpp>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

class PacketBuilder {
public:
  // Add preamble
  PacketBuilder& preamble(size_t count = MDU_TX_MIN_PREAMBLE_BITS);

  // Add command
  PacketBuilder& command(mdu::Command cmd);

  // Add data
  PacketBuilder& data(std::unsigned_integral auto... values) {
    return (data(values), ...);
  }

  PacketBuilder& data(std::unsigned_integral auto value) {
    for (auto i{sizeof(value)}; i-- > 0uz;)
      _packet.push_back(static_cast<uint8_t>(value >> i * CHAR_BIT));
    return *this;
  }

  PacketBuilder& data(std::span<uint8_t const> bytes);
  PacketBuilder& data(std::string_view sv);

  // CRC
  PacketBuilder& crc8(std::optional<uint8_t> overwrite_crc8 = {});
  PacketBuilder& crc32(std::optional<uint32_t> overwrite_crc32 = {});

  // Add ackreq bits
  PacketBuilder& ackreq(size_t count = 10uz);

  // Get Packet
  mdu::Packet packet() const;

  // Get timings
  mdu::tx::Timings
  timings(mdu::TransferRate transfer_rate = mdu::TransferRate::Default) const;

  // Get timings without ackreq
  mdu::tx::Timings timingsWithoutAckreq(
    mdu::TransferRate transfer_rate = mdu::TransferRate::Default) const;

  // Get ackreq timings
  mdu::tx::Timings timingsAckreqOnly(
    size_t count = MDU_TX_MIN_ACKREQ_BITS,
    mdu::TransferRate transfer_rate = mdu::TransferRate::Default) const;

  // Builder methods
  static PacketBuilder makePingPacket(uint8_t decoder_id);
  static PacketBuilder makePingPacket(uint32_t serial_number,
                                      uint32_t decoder_id);
  static PacketBuilder
  makeConfigTransferRatePacket(mdu::TransferRate transfer_rate);
  static PacketBuilder makeBusyPacket();
  static PacketBuilder
  makeZsuUpdatePacket(uint32_t addr, std::span<uint8_t const, 64uz> bytes);
  static PacketBuilder makeZppValidQueryPacket(std::string_view zpp_id,
                                               size_t zpp_flash_size);
  static PacketBuilder
  makeZppLcDcQueryPacket(std::span<uint8_t const, 4uz> developer_code);
  static PacketBuilder makeZppUpdatePacket(uint32_t addr,
                                           std::span<uint8_t const> bytes);
  static PacketBuilder makeZppUpdateEndPacket(uint32_t begin_addr,
                                              uint32_t end_addr);
  static PacketBuilder makeZppExitPacket();

private:
  size_t _preamble_count{};
  size_t _ackreq_count{};
  mdu::Packet _packet{};
};
