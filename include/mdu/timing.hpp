// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Timing
///
/// \file   mdu/timing.hpp
/// \author Vincent Hamp
/// \date   26/01/2023

#pragma once

#include <array>
#include <cstddef>
#include "bit.hpp"

namespace mdu {

/// Timing in µs for all bits including closed-interval for upper- an
/// lowerbounds
struct Timing {
  uint16_t one_min{};
  uint16_t one{};
  uint16_t one_max{};
  uint16_t zero_min{};
  uint16_t zero{};
  uint16_t zero_max{};
  uint16_t ackreq_min{};
  uint16_t ackreq{};
  uint16_t ackreq_max{};
  uint16_t ack{};
};

/// Make timing and set upper- and lowerbounds according to tolerance
///
/// \param  one       Onebit timing
/// \param  zero      Zerobit timing
/// \param  ackreq    Ackreqbit timing
/// \param  ack       Ackbit timing
/// \param  tolerance Tolerance for lower- and upperbounds
/// \return Timing
consteval Timing make_timing(uint16_t one,
                             uint16_t zero,
                             uint16_t ackreq,
                             uint16_t ack,
                             double tolerance) {
  return {.one_min = static_cast<uint16_t>(one * (1.0 - tolerance)),
          .one = one,
          .one_max = static_cast<uint16_t>(one * (1.0 + tolerance)),
          .zero_min = static_cast<uint16_t>(zero * (1.0 - tolerance)),
          .zero = zero,
          .zero_max = static_cast<uint16_t>(zero * (1.0 + tolerance)),
          .ackreq_min = static_cast<uint16_t>(ackreq * (1.0 - tolerance)),
          .ackreq = ackreq,
          .ackreq_max = static_cast<uint16_t>(ackreq * (1.0 + tolerance)),
          .ack = ack};
}

/// Array of timings (index equals set transfer rate)
inline constexpr std::array<Timing, 5uz> timings{
  make_timing(1200u, 2400u, 3600u, 100u, 0.1),
  make_timing(10u, 20u, 60u, 40u, 0.3),
  make_timing(20u, 40u, 60u, 40u, 0.2),
  make_timing(40u, 80u, 120u, 80u, 0.2),
  make_timing(75u, 150u, 225u, 100u, 0.1)};

/// Fallback time
inline constexpr auto& fallback_timing{timings[0uz]};

/// Check if time is a fallback one bit
///
/// \param  time  Time in µs
/// \return true  Time equals fallback one bit
/// \return false Time does not equal fallback one bit
constexpr bool is_fallback_one(uint32_t time) {
  return (time >= fallback_timing.one_min && time <= fallback_timing.one_max);
}

/// Check if time is a one bit
///
/// \param  time                Time is µs
/// \param  transfer_rate_index Current index of transfer rate
/// \return true                Time equals one bit
/// \return false               Time does not equal one bit
constexpr bool is_one(uint32_t time, size_t transfer_rate_index) {
  auto const i{transfer_rate_index};
  return (time >= timings[i].one_min && time <= timings[i].one_max) ||
         is_fallback_one(time);
}

/// Check if time is a fallback zero bit
///
/// \param  time  Time in µs
/// \return true  Time equals fallback zero bit
/// \return false Time does not equal fallback zero bit
constexpr bool is_fallback_zero(uint32_t time) {
  return (time >= fallback_timing.zero_min && time <= fallback_timing.zero_max);
}

/// Check if time is a zero bit
///
/// \param  time                Time is µs
/// \param  transfer_rate_index Current index of transfer rate
/// \return true                Time equals zero bit
/// \return false               Time does not equal zero bit
constexpr bool is_zero(uint32_t time, size_t transfer_rate_index) {
  auto const i{transfer_rate_index};
  return (time >= timings[i].zero_min && time <= timings[i].zero_max) ||
         is_fallback_zero(time);
}

/// Check if time is a fallback ackreq bit
///
/// \param  time  Time in µs
/// \return true  Time equals fallback ackreq bit
/// \return false Time does not equal fallback ackreq bit
constexpr bool is_fallback_ackreq(uint32_t time) {
  return (time >= fallback_timing.ackreq_min &&
          time <= fallback_timing.ackreq_max);
}

/// Check if time is a ackreq bit
///
/// \param  time                Time is µs
/// \param  transfer_rate_index Current index of transfer rate
/// \return true                Time equals ackreq bit
/// \return false               Time does not equal ackreq bit
constexpr bool is_ackreq(uint32_t time, size_t transfer_rate_index) {
  auto const i{transfer_rate_index};
  return (time >= timings[i].ackreq_min && time <= timings[i].ackreq_max) ||
         is_fallback_ackreq(time);
}

/// Convert time to bit
///
/// \param  time                Time is µs
/// \param  transfer_rate_index Current index of transfer rate
/// \return _0                  Time is a zero bit
/// \return _1                  Time is a one bit
/// \return Ackreq              Time is a ackreq bit
/// \return Invalid             Time isn't valid
constexpr Bit time2bit(uint32_t time, size_t transfer_rate_index) {
  if (is_zero(time, transfer_rate_index)) return _0;
  else if (is_one(time, transfer_rate_index)) return _1;
  else if (is_ackreq(time, transfer_rate_index)) return Ackreq;
  else return Invalid;
}

}  // namespace mdu