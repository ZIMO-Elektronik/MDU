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
  PacketBuilder& preamble(size_t count = MDU_TX_PREAMBLE_BITS);

  // Add command
  PacketBuilder& command(mdu::Command cmd);

  // Add data
  PacketBuilder& data(std::unsigned_integral auto... values) {
    return (data(values), ...);
  }

  PacketBuilder& data(std::unsigned_integral auto value) {
    for (auto i{sizeof(value)}; i-- > 0uz;)
      data_.push_back(static_cast<uint8_t>(value >> i * CHAR_BIT));
    return *this;
  }

  PacketBuilder& data(std::span<uint8_t const> chunk);
  PacketBuilder& data(std::string_view sv);

  // CRC
  PacketBuilder& crc8(std::optional<uint8_t> overwrite_crc8 = {});
  PacketBuilder& crc32(std::optional<uint32_t> overwrite_crc32 = {});

  // Add ackreq bits
  PacketBuilder& ackreq(size_t count = 10uz);

  // Get timings
  std::vector<uint32_t>
  timings(mdu::TransferRate transfer_rate = mdu::TransferRate::Default) const;

  // Get timings without ackreq
  std::vector<uint32_t> timingsWithoutAckreq(
    mdu::TransferRate transfer_rate = mdu::TransferRate::Default) const;

  // Get ackreq timings
  std::vector<uint32_t> timingsAckreqOnly(
    size_t count = MDU_TX_ACKREQ_BITS,
    mdu::TransferRate transfer_rate = mdu::TransferRate::Default) const;

private:
  size_t preamble_count_{};
  size_t ackreq_count_{};
  std::vector<uint8_t> data_{};
};