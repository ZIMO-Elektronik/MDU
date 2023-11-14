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
      uint32_t const tmp{_crc};
      _crc <<= 1u;
      if (byte & 0x80u) _crc |= 1u;
      if (tmp & 0x8000'0000u) _crc ^= 0x4C11DB7u;
      byte = static_cast<uint8_t>(byte << 1u);
    }
  }

  constexpr void next(std::span<uint8_t const> bytes) {
    std::ranges::for_each(bytes, [this](uint8_t byte) { next(byte); });
  }

  constexpr uint32_t value() {
    auto const cpy{_crc};
    constexpr std::array<uint8_t, 4uz> zeros{};
    next(zeros);
    auto const retval{_crc};
    _crc = cpy;
    return retval;
  }

  constexpr operator uint32_t() { return value(); }
};

static_assert(sizeof(Crc32) == sizeof(uint32_t));

/// "CRC32" with no polynomial representation whatsoever...
///
/// \param  bytes Bytes to calculate "CRC32" for
/// \return CRC32
constexpr uint32_t crc32(std::span<uint8_t const> bytes) {
  Crc32 crc;
  crc.next(bytes);
  return crc;
}

}  // namespace mdu