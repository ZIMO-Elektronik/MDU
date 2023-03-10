#include "packet_builder.hpp"
#include <algorithm>

PacketBuilder& PacketBuilder::preamble(size_t count) {
  preamble_count_ = count;
  return *this;
}

PacketBuilder& PacketBuilder::command(mdu::Command cmd) {
  return data(std::to_underlying(cmd));
}

PacketBuilder& PacketBuilder::data(std::span<uint8_t const> chunk) {
  data_.insert(end(data_), cbegin(chunk), cend(chunk));
  return *this;
}

PacketBuilder& PacketBuilder::data(std::string_view sv) {
  data_.insert(end(data_), cbegin(sv), cend(sv));
  return *this;
}

PacketBuilder& PacketBuilder::crc8(std::optional<uint8_t> overwrite_crc8) {
  return data(overwrite_crc8.value_or(mdu::crc8(data_)));
}

PacketBuilder& PacketBuilder::crc32(std::optional<uint32_t> overwrite_crc32) {
  return data(overwrite_crc32.value_or(mdu::crc32(data_)));
}

PacketBuilder& PacketBuilder::ackreq(size_t count) {
  ackreq_count_ = count;
  return *this;
}

std::vector<uint32_t>
PacketBuilder::timings(mdu::TransferRate transfer_rate) const {
  std::vector<uint32_t> retval{timingsWithoutAckreq(transfer_rate)};
  auto const& timings{mdu::timings[std::to_underlying(transfer_rate)]};

  // Ackreq
  for (auto i{0uz}; i < ackreq_count_; ++i)
    retval.push_back(timings.ackreq);

  return retval;
}

std::vector<uint32_t>
PacketBuilder::timingsWithoutAckreq(mdu::TransferRate transfer_rate) const {
  std::vector<uint32_t> retval;
  auto const& timings{mdu::timings[std::to_underlying(transfer_rate)]};

  // Preamble
  for (auto i{0uz}; i < preamble_count_; ++i)
    retval.push_back(timings.one);

  // Data
  std::ranges::for_each(data_, [&](uint8_t byte) {
    retval.push_back(timings.zero);  // Start
    for (auto i{sizeof(byte) * CHAR_BIT}; i-- > 0uz;)
      retval.push_back(byte & (1u << i) ? timings.one : timings.zero);
  });

  // End
  retval.push_back(timings.one);

  return retval;
}

std::vector<uint32_t>
PacketBuilder::timingsAckreqOnly(size_t count,
                                 mdu::TransferRate transfer_rate) const {
  return std::vector<uint32_t>(
    count, mdu::timings[std::to_underlying(transfer_rate)].ackreq);
}