// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Command station
///
/// \file   mdu/tx/command_station.hpp
/// \author Jonas Gahlert
/// \date   10/03/2025

#pragma once

#include "ackreq.hpp"
#include "track_outputs.hpp"

namespace mdu::tx {

template<typename T>
concept CommandStation = Ackreq<T> && TrackOutputs<T>;

} // namespace mdu::tx
