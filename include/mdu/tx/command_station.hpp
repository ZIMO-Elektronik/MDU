#pragma once

#include <concepts>

namespace mdu::tx {

template<typename T>
concept CommandStation = requires(T t) {
  { t.ackreqBegin() } -> std::same_as<void>;
  { t.ackreqNext() } -> std::same_as<void>;
  { t.ackreqEnd() } -> std::same_as<void>;
};

} // namespace mdu::tx