// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Receive entry point
///
/// \file   mdu/rx/entry/point.hpp
/// \author Vincent Hamp
/// \date   12/12/2022

#pragma once

#include <array>
#include <cstdint>
#include <ztl/inplace_deque.hpp>
#include "config.hpp"

namespace mdu::rx::entry {

/// Entry point with CV verify
struct Point {
  Point(Config cfg) : _cfg{cfg} {}
  void verify(uint32_t cv_addr, uint8_t byte);

private:
  Config const _cfg;
  std::array<std::pair<uint8_t, uint8_t>, 4uz> const _sn_sequence{
    {{105u - 1u, static_cast<uint8_t>(_cfg.serial_number >> 24u)},
     {106u - 1u, static_cast<uint8_t>(_cfg.serial_number >> 16u)},
     {105u - 1u, static_cast<uint8_t>(_cfg.serial_number >> 8u)},
     {106u - 1u, static_cast<uint8_t>(_cfg.serial_number >> 0u)}}};
  std::array<std::pair<uint8_t, uint8_t>, 4uz> const _id_sequence{
    {{105u - 1u, static_cast<uint8_t>(_cfg.decoder_id >> 24u)},
     {106u - 1u, static_cast<uint8_t>(_cfg.decoder_id >> 16u)},
     {105u - 1u, static_cast<uint8_t>(_cfg.decoder_id >> 8u)},
     {106u - 1u, static_cast<uint8_t>(_cfg.decoder_id >> 0u)}}};
  ztl::inplace_deque<std::pair<uint8_t, uint8_t>, 9uz> _deque{};
};

} // namespace mdu::rx::entry
