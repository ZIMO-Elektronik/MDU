// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive entry point
///
/// \file   rx/entry/point.cpp
/// \author Vincent Hamp
/// \date   12/12/2022

#include "rx/entry/point.hpp"

namespace mdu::rx::entry {

namespace {

/// Make first half of ZSU entry sequence
///
/// \param  decoder_id  Decoder ID
/// \return First half of ZSU entry sequence
constexpr auto make_zsu_sequence(uint32_t decoder_id) {
  return std::array<std::pair<uint8_t, uint8_t>, 5uz>{
    {{8u - 1u, 255u},
     {105u - 1u, (decoder_id & 0xFF00'0000u) >> 24u},
     {106u - 1u, (decoder_id & 0x00FF'0000u) >> 16u},
     {105u - 1u, (decoder_id & 0x0000'FF00u) >> 8u},
     {106u - 1u, (decoder_id & 0x0000'00FFu) >> 0u}}};
}

/// Make first half of ZPP entry sequence
///
/// \return First half of ZPP entry sequence
consteval auto make_zpp_sequence() {
  return std::array<std::pair<uint8_t, uint8_t>, 5uz>{{{8u - 1u, 254u},
                                                       {105u - 1u, 0xAAu},
                                                       {106u - 1u, 0x55u},
                                                       {105u - 1u, 0x55u},
                                                       {106u - 1u, 0xAAu}}};
}

/// Make second half of ZPP/ZSU entry
///
/// \return Second half of ZPP/ZSU entry
consteval auto make_zero_sequence() {
  return std::array<std::pair<uint8_t, uint8_t>, 4uz>{
    {{105u - 1u, 0u}, {106u - 1u, 0u}, {105u - 1u, 0u}, {106u - 1u, 0u}}};
}

/// Index is what we are looking for
///
/// \param  index Index to check
/// \return true  Right index
/// \return false Wrong index
constexpr bool is_index(size_t index) {
  return index == 8uz - 1uz || index == 105uz - 1uz || index == 106uz - 1uz;
}

}  // namespace

/// Verify
///
/// \param  index CV index
/// \param  value CV value
void Point::verify(size_t index, uint8_t value) {
  if (!is_index(index)) return _deque.clear();
  if (size(_deque) && _deque.back().first == index) return;
  _deque.push_back({static_cast<uint8_t>(index), value});
  if (_deque.front().second == 0xFFu)
    verifySequence(make_zsu_sequence(_cfg.decoder_id), _cfg.zsu_entry);
  else verifySequence(make_zpp_sequence(), _cfg.zpp_entry);
}

/// Verify entry sequence
///
/// \param  sequence  Sequence to verify
/// \param  f         Function to call in case sequence matches
void Point::verifySequence(
  std::array<std::pair<uint8_t, uint8_t>, 5uz> const& sequence,
  std::function<void()> const& f) {
  static constexpr auto zero_sequence{make_zero_sequence()};
  auto const n{size(_deque)};
  if (!((n <= 5uz && std::equal(begin(_deque), end(_deque), begin(sequence))) ||
        (n > 5uz &&
         std::equal(begin(_deque) + 5, end(_deque), begin(zero_sequence)))))
    _deque.clear();
  else if (n == _deque.max_size()) std::invoke(f);
}

}  // namespace mdu::rx::entry