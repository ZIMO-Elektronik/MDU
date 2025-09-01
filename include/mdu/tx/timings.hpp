// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Transmit timings
///
/// \file   mdu/tx/timings.hpp
/// \author Jonas Gahlert
/// \date   10.06.2025

#pragma once

#include <cstdint>
#include <span>
#include <ztl/inplace_vector.hpp>
#include "../packet.hpp"
#include "../timing.hpp"
#include "config.hpp"

namespace mdu::tx {

using Timings =
  ztl::inplace_vector<uint16_t,
                      MDU_TX_MAX_PREAMBLE_BITS                 // Preamble
                        + MDU_MAX_PACKET_SIZE*(CHAR_BIT + 1uz) // Data
                        + 1uz>;                                // End

/// Convert bytes into timings
///
/// \param  bytes Bytes
/// \param  cfg   Configuration
/// \param  rate  Transfer rate
/// \return Timings
constexpr Timings bytes2timings(std::span<uint8_t const> bytes,
                                Config cfg = {},
                                TransferRate rate = TransferRate::Default) {
  Timings result{};
  auto first{begin(result)};

  auto const& timings{mdu::timings[std::to_underlying(rate)]};

  // Preamble
  first =
    std::ranges::fill_n(first,
                        static_cast<Timings::difference_type>(cfg.num_preamble),
                        timings.one);

  // Data
  std::ranges::for_each(bytes, [&](uint8_t byte) {
    *first++ = timings.zero; // Start
    for (auto i{sizeof(byte) * CHAR_BIT}; i-- > 0uz;)
      *first++ = (byte & (1u << i) ? timings.one : timings.zero);
  });

  // End
  if (size(bytes)) *first++ = timings.one;

  // Ackreq
  first =
    std::ranges::fill_n(first,
                        static_cast<Timings::difference_type>(cfg.num_ackreq),
                        timings.ackreq);

  // Size
  result.resize(static_cast<Timings::size_type>(
    std::ranges::distance(cbegin(result), first)));

  return result;
}

/// Convert packet into timings
///
/// \param  bytes Bytes
/// \param  cfg   Configuration
/// \param  rate  Transfer rate
/// \return Timings
constexpr Timings packet2timings(Packet const& packet,
                                 Config cfg = {},
                                 TransferRate rate = TransferRate::Default) {
  return bytes2timings({cbegin(packet), size(packet)}, cfg, rate);
}

} // namespace mdu::tx
