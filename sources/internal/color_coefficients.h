#pragma once

#include <weqeqq/color.h>

#include <cstdint>

namespace weqeqq::color::internal {

template <Standard Std>
struct Coeff;

template <>
struct Coeff<Standard::kBt601> {
  static constexpr std::uint16_t kR = 76;
  static constexpr std::uint16_t kG = 150;
  static constexpr std::uint16_t kB = 29;
};

template <>
struct Coeff<Standard::kBt709> {
  static constexpr std::uint16_t kR = 54;
  static constexpr std::uint16_t kG = 183;
  static constexpr std::uint16_t kB = 18;
};

}  // namespace weqeqq::color::internal
