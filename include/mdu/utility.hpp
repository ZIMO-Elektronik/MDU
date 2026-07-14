// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Utility
///
/// \file   mdu/utility.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <concepts>
#include <cstdint>
#include <span>

extern "C" {
#include <salsa20/ecrypt-sync.h>
}

namespace mdu {

ECRYPT_ctx make_salsa20_context(uint32_t decoder_id,
                                std::span<uint8_t const, 8uz> iv,
                                char const* master_key);

/// Data to uint16_t
///
/// \tparam RandomIt  std::random_access_iterator
/// \param  first     Beginning of the source range
/// \return uint16_t
template<std::random_access_iterator RandomIt>
requires(sizeof(std::iter_value_t<RandomIt>) == 1uz)
constexpr uint16_t data2uint16(RandomIt first) {
  return static_cast<uint16_t>(static_cast<uint32_t>(first[0uz]) << 8u |
                               static_cast<uint32_t>(first[1uz]) << 0u);
}

/// Data to uint32_t
///
/// \tparam RandomIt  std::random_access_iterator
/// \param  first     Beginning of the source range
/// \return uint32_t
template<std::random_access_iterator RandomIt>
requires(sizeof(std::iter_value_t<RandomIt>) == 1uz)
constexpr uint32_t data2uint32(RandomIt first) {
  return static_cast<uint32_t>(first[0uz]) << 24u |
         static_cast<uint32_t>(first[1uz]) << 16u |
         static_cast<uint32_t>(first[2uz]) << 8u |
         static_cast<uint32_t>(first[3uz]) << 0u;
}

/// Data to uint64_t
///
/// \tparam RandomIt  std::random_access_iterator
/// \param  first     Beginning of the source range
/// \return uint64_t
template<std::random_access_iterator RandomIt>
requires(sizeof(std::iter_value_t<RandomIt>) == 1uz)
constexpr uint64_t data2uint64(RandomIt first) {
  auto const upper{data2uint32(first)};
  auto const lower{data2uint32(first + sizeof(uint32_t))};
  return static_cast<uint64_t>(upper) << 32u | lower;
}

/// uint16 to data
///
/// \tparam OutputIt  std::output_iterator
/// \param  hword     Half-word to convert
/// \param  out       Beginning of the destination range
/// \return Output iterator one past the last element copied
template<std::output_iterator<uint8_t> OutputIt>
constexpr auto uint16_2data(uint16_t hword, OutputIt out) {
  *out++ = static_cast<uint8_t>(hword >> 8u);
  *out++ = static_cast<uint8_t>(hword >> 0u);
  return out;
}

/// uint32_t to data
///
/// \tparam OutputIt  std::output_iterator
/// \param  word      Word to convert
/// \param  out       Beginning of the destination range
/// \return Output iterator one past the last element copied
template<std::output_iterator<uint8_t> OutputIt>
constexpr auto uint32_2data(uint32_t word, OutputIt out) {
  *out++ = static_cast<uint8_t>(word >> 24u);
  *out++ = static_cast<uint8_t>(word >> 16u);
  *out++ = static_cast<uint8_t>(word >> 8u);
  *out++ = static_cast<uint8_t>(word >> 0u);
  return out;
}

} // namespace mdu
