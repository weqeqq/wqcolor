#pragma once

#include "internal/color_types.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <source_location>
#include <span>
#include <string_view>

namespace weqeqq::color::internal {

inline bool ContractViolation(
    std::string_view message,
    std::source_location location = std::source_location::current())
    noexcept(!kDebug) {
  if constexpr (kDebug) {
    throw Error(message, location);
  }

  return false;
}

inline bool ValidateFormatArgument(
    Format format, std::string_view message,
    std::source_location location = std::source_location::current())
    noexcept(!kDebug) {
  if (IsValidFormat(format)) {
    return true;
  }

  return ContractViolation(message, location);
}

inline bool ValidateStandardArgument(
    Standard standard, std::string_view message,
    std::source_location location = std::source_location::current())
    noexcept(!kDebug) {
  if (IsValidStandard(standard)) {
    return true;
  }

  return ContractViolation(message, location);
}

inline bool SpansOverlap(std::span<const std::uint8_t> input,
                         std::span<std::uint8_t> output) noexcept {
  if (input.empty() || output.empty()) {
    return false;
  }

  const auto input_address = reinterpret_cast<std::uintptr_t>(input.data());
  const auto output_address = reinterpret_cast<std::uintptr_t>(output.data());
  if (input_address <= output_address) {
    return output_address - input_address < input.size();
  }

  return input_address - output_address < output.size();
}

inline bool ValidateConvertArguments(
    std::span<const std::uint8_t> input, std::span<std::uint8_t> output,
    Format input_color, Format output_color, Standard standard,
    std::source_location location = std::source_location::current())
    noexcept(!kDebug) {
  if (!ValidateFormatArgument(input_color, "input format is invalid",
                              location) ||
      !ValidateFormatArgument(output_color, "output format is invalid",
                              location) ||
      !ValidateStandardArgument(standard, "standard is invalid", location)) {
    return false;
  }

  if (SpansOverlap(input, output)) {
    return ContractViolation("input and output spans must not overlap",
                             location);
  }

  return true;
}

inline std::optional<ConvertContext> PrepareConvertContext(
    std::span<const std::uint8_t> input, std::span<std::uint8_t> output,
    Format input_color, Format output_color) noexcept {
  if (input.empty() || output.empty()) {
    return std::nullopt;
  }

  const auto input_channels = ChannelCount(input_color);
  const auto output_channels = ChannelCount(output_color);
  const auto input_pixels = input.size() / input_channels;
  const auto output_pixels = output.size() / output_channels;
  const auto pixel_count = std::min(input_pixels, output_pixels);
  if (pixel_count == 0) {
    return std::nullopt;
  }

  return ConvertContext{
      .input = input.data(),
      .output = output.data(),
      .pixel_count = pixel_count,
  };
}

constexpr std::size_t DispatchTableIndex(Format input_color, Format output_color,
                                         Standard standard) noexcept {
  return EnumIndex(input_color) * (kFormatCount * kStandardCount) +
         EnumIndex(output_color) * kStandardCount + EnumIndex(standard);
}

}  // namespace weqeqq::color::internal
