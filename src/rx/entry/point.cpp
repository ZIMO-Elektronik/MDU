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

/// Make first half of ZPP entry sequence
///
/// \return First half of ZPP entry sequence
consteval auto make_zpp_sequence() {
  return std::array<std::pair<uint8_t, uint8_t>, 4uz>{{{105u - 1u, 0xAAu},
                                                       {106u - 1u, 0x55u},
                                                       {105u - 1u, 0x55u},
                                                       {106u - 1u, 0xAAu}}};
}

/// Make half of ZPP/ZSU zero entry sequence
///
/// \return Half of ZPP/ZSU zero entry sequence
consteval auto make_zero_sequence() {
  return std::array<std::pair<uint8_t, uint8_t>, 4uz>{{{105u - 1u, 0x00u},
                                                       {106u - 1u, 0x00u},
                                                       {105u - 1u, 0x00u},
                                                       {106u - 1u, 0x00u}}};
}

/// Index is what we are looking for
///
/// \param  cv_addr CV address to check
/// \retval true    Right CV address
/// \retval false   Wrong CV address
constexpr bool is_cv_addr(uint32_t cv_addr) {
  return cv_addr == 8uz - 1uz || cv_addr == 105uz - 1uz ||
         cv_addr == 106uz - 1uz;
}

}  // namespace

/// Verify
///
/// \param  cv_addr CV address
/// \param  byte    CV value
void Point::verify(uint32_t cv_addr, uint8_t byte) {
  // Not what we're looking for
  if (!is_cv_addr(cv_addr)) return _deque.clear();
  // Repetitions
  else if (size(_deque) &&
           _deque.back() == std::pair<uint8_t, uint8_t>{cv_addr, byte})
    return;

  _deque.push_back({static_cast<uint8_t>(cv_addr), byte});
  auto const n{size(_deque)};

  static constexpr auto zpp_sequence{make_zpp_sequence()};
  static constexpr auto zero_sequence{make_zero_sequence()};

  // ZPP
  if (_deque.front() == std::pair<uint8_t, uint8_t>{8u - 1u, 0xFEu}) {
    if ((n <= 5uz &&  // First four must always be 0xAA, 0x55, 0x55, 0xAA
         std::equal(cbegin(_deque) + 1, cend(_deque), cbegin(zpp_sequence))) ||
        (n > 5uz &&  // Second four could be SN or 0x00
         (std::equal(begin(_deque) + 5, end(_deque), begin(_sn_sequence)) ||
          std::equal(begin(_deque) + 5, end(_deque), begin(zero_sequence))))) {
      if (n == _deque.max_size()) std::invoke(_cfg.zpp_entry);
      return;
    }
  }
  // ZSU
  else if (_deque.front() == std::pair<uint8_t, uint8_t>{8u - 1u, 0xFFu}) {
    if ((n <= 5uz &&  //
         (std::equal(begin(_deque) + 1, end(_deque), begin(_id_sequence)) ||
          std::equal(begin(_deque) + 1, end(_deque), begin(zero_sequence)))) ||
        (n > 5uz &&  // Second four could be SN or 0x00
         (std::equal(begin(_deque) + 5, end(_deque), begin(_sn_sequence)) ||
          std::equal(begin(_deque) + 5, end(_deque), begin(zero_sequence))))) {
      if (n == _deque.max_size()) std::invoke(_cfg.zsu_entry);
      return;
    }
  }

  _deque.clear();
}

}  // namespace mdu::rx::entry