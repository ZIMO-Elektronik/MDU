// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// CRC32
///
/// \file   mdu/crc32.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <array>
#include <climits>
#include "crc8.hpp"

namespace mdu {

/// "CRC32" with no polynomial representation whatsoever...
struct Crc32 : detail::CrcBase<uint32_t, static_cast<uint32_t>(-1)> {
  constexpr void next(uint8_t byte) {
    for (auto i{0}; i < CHAR_BIT; ++i) {
      uint32_t const tmp{crc_};
      crc_ <<= 1u;
      if (byte & 0x80u) crc_ |= 1u;
      if (tmp & 0x8000'0000u) crc_ ^= 0x4C11DB7u;
      byte <<= 1u;
    }
  }

  constexpr void next(std::span<uint8_t const> chunk) {
    std::ranges::for_each(chunk, [this](uint8_t byte) { next(byte); });
  }

  constexpr uint32_t value() {
    auto const cpy{crc_};
    constexpr std::array<uint8_t, 4uz> zeros{};
    next(zeros);
    auto const retval{crc_};
    crc_ = cpy;
    return retval;
  }

  constexpr operator uint32_t() { return value(); }
};

/// "CRC32" with no polynomial representation whatsoever...
///
/// \param  chunk Chunk to calculate "CRC32" for
/// \return CRC32
constexpr uint32_t crc32(std::span<uint8_t const> chunk) {
  Crc32 crc;
  crc.next(chunk);
  return crc;
}

}  // namespace mdu