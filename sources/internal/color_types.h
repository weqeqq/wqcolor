#pragma once

#include <weqeqq/color.h>

#include <cstddef>
#include <cstdint>

namespace weqeqq::color::internal {

using ConvertFn = void (*)(const std::uint8_t* input, std::uint8_t* output,
                           std::size_t pixel_count,
                           parallel::ExecutionPolicy execution);

struct ConvertContext {
  const std::uint8_t* input;
  std::uint8_t* output;
  std::size_t pixel_count;
};

inline constexpr std::size_t kFormatCount = static_cast<std::size_t>(Format::kCount);
inline constexpr std::size_t kStandardCount =
    static_cast<std::size_t>(Standard::kCount);
inline constexpr std::size_t kDispatchTableSize =
    kFormatCount * kFormatCount * kStandardCount;

template <typename Enum>
constexpr std::size_t EnumIndex(Enum value) noexcept {
  return static_cast<std::size_t>(value);
}

}  // namespace weqeqq::color::internal
